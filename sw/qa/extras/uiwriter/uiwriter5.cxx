/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/settings.hxx>
#include <ndtxt.hxx>
#include <wrtsh.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <flyfrm.hxx>
#include <fmtanchr.hxx>
#include <UndoManager.hxx>
#include <sortedobjs.hxx>
#include <xmloff/odffields.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <comphelper/lok.hxx>
#include <txtfrm.hxx>
#include <tabfrm.hxx>
#include <view.hxx>
#include <cmdid.h>
#include <AnnotationWin.hxx>
#include <PostItMgr.hxx>
#include <frameformats.hxx>
#include <tools/json_writer.hxx>
#include <unotxdoc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest5 : public SwModelTestBase
{
public:
    SwUiWriterTest5()
        : SwModelTestBase(u"/sw/qa/extras/uiwriter/data/"_ustr)
    {
    }

protected:
    AllSettings m_aSavedSettings;
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf139127)
{
    createSwDoc("tdf139127.fodt");
    SwDoc* pDoc = getSwDoc();

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell->GetViewOptions()->IsShowChangesInMargin());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert
                                                      | RedlineFlags::ShowDelete);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // two pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // delete the last two characters with a page break at the end of the document
    dispatchCommand(mxComponent, u".uno:GoToEndOfDoc"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SwBackspace"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"First page"_ustr, xTextDocument->getText()->getString());

    // Undo
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    // this would crash due to bad redline range
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(u"First page"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"B"_ustr, getParagraph(2)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf138479)
{
    createSwDoc();
    SwDoc* const pDoc = getSwDoc();

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    pWrtShell->Insert(u"Lorem"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"Lorem"_ustr, getParagraph(1)->getString());

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines shouldn't be visible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    // delete "r" in "Lorem"
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Loem"_ustr, getParagraph(1)->getString());

    // delete "oe" in "Loem"
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Lm"_ustr, getParagraph(1)->getString());

    // test embedded Undo in ChangesInMargin mode
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Loem"_ustr, getParagraph(1)->getString());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(u"Lorem"_ustr, getParagraph(1)->getString());

    // this would crash due to bad redline range
    for (int i = 0; i < 5; ++i)
        dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf138666)
{
    createSwDoc("tdf39721.fodt");
    SwDoc* pDoc = getSwDoc();

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // show deletions inline
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"dolor sit"_ustr, getParagraph(2)->getString());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    // show deletions in margin
    CPPUNIT_ASSERT_EQUAL(u"Loremm"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"dolsit"_ustr, getParagraph(2)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    // show deletions inline again
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"dolor sit"_ustr, getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf140982)
{
    createSwDoc("tdf115815.odt");
    SwDoc* pDoc = getSwDoc();

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // show deletions inline
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum dolor sit amet..."_ustr, getParagraph(1)->getString());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    // show deletions in margin
    CPPUNIT_ASSERT_EQUAL(u"Lorem  amet..."_ustr, getParagraph(1)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, u".uno:ShowChangesInMargin"_ustr, {});

    // show deletions inline again
    CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum dolor sit amet..."_ustr, getParagraph(1)->getString());

    // Save it and load it back.
    saveAndReload(u"writer8"_ustr);

    // Test comment range feature on tracked deletion.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(),
                                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    bool bAnnotationStart = false;
    bool bBeforeAnnotation = true;
    OUString sTextBeforeAnnotation;
    while (xRunEnum->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
        OUString aType = getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr);
        // there is no AnnotationEnd with preceding AnnotationStart,
        // i.e. annotation with lost range
        CPPUNIT_ASSERT(aType != "AnnotationEnd" || !bAnnotationStart);

        bAnnotationStart = (aType == "Annotation");

        // collect paragraph text before the first annotation
        if (bBeforeAnnotation)
        {
            if (bAnnotationStart)
                bBeforeAnnotation = false;
            else if (aType == "Text")
            {
                uno::Reference<text::XTextRange> xRun(xPropertySet, uno::UNO_QUERY);
                sTextBeforeAnnotation += xRun->getString();
            }
        }
    }

    // This was "Lorem ipsum" (collapsed annotation range)
    CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, sTextBeforeAnnotation);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf126206)
{
    createSwDoc("tdf126206.docx");

    // normal text (it was bold)
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 4)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"ipsum"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 3)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"ipsum"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf50447)
{
    createSwDoc("tdf126206.docx");

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }

    // remove bold formatting with change tracking
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Bold"_ustr, {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, xCursor->getString());
        // This was NORMAL
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143918)
{
    createSwDoc("tdf126206.docx");

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }

    // remove bold formatting with change tracking and after that, apply underline, too
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Bold"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Underline"_ustr, {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, xCursor->getString());
        // This was NORMAL (only underlining was removed)
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143938)
{
    createSwDoc("tdf54819.fodt");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    // select first paragraph, add underline without change tracking
    pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, u".uno:Underline"_ustr, {});

    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));
    }

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // apply italic with change tracking
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Italic"_ustr, {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        // (first empty run is associated to the redline)
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // no italic, but still underline direct formatting
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem ipsum"_ustr, xCursor->getString());
        // This wasn't underlined (lost direct formatting)
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143939)
{
    createSwDoc("tdf126206.docx");

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }

    // positionate the text cursor inside the first word
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // remove bold formatting with change tracking without selection
    dispatchCommand(mxComponent, u".uno:Bold"_ustr, {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem"_ustr, xCursor->getString());
        // This was NORMAL
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(xCursor, u"CharWeight"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf50447_keep_hints)
{
    createSwDoc("tdf50447.fodt");
    SwDoc* pDoc = getSwDoc();

    // first paragraph (_Lorem_ /ipsum/)

    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u" "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"ipsum"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));
    }

    // second paragraph (_dolor_ sit /amet/.)

    xText = getParagraph(2)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(2), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"dolor"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u" sit "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"amet"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));
    }

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // modify character formatting of the all the text

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:SuperScript"_ustr, {});

    // multiple format redlines for the multiple hints

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    // This was 1.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(7), pEditShell->GetRedlineCount());

    // reject tracked changes

    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // all hints and text portions between them got back the original formatting

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"Lorem"_ustr, xCursor->getString());
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u" "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"ipsum"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));
    }

    // second paragraph (_dolor_ sit /amet/.)

    xText = getParagraph(2)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(2), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"dolor"_ustr, xCursor->getString());
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u" sit "_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(u"amet"_ustr, xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, u"CharUnderline"_ustr));
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, u"CharPosture"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf144272)
{
    createSwDoc("tdf50447.fodt");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* const pWrtShell = getSwDocShell()->GetWrtShell();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // paragraph style of the first paragraph: Heading 1, second paragraph: Standard

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    // modify with track changes: Standard and Heading 2

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Standard"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);
    pWrtShell->Down(/*bSelect=*/false);
    aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(u"Heading 2"_ustr) },
        { "FamilyName", uno::Any(u"ParagraphStyles"_ustr) },
    });
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr, aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    // reject tracked changes
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    // This was Standard (missing reject)
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    // This was Heading 2 (missing reject)
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf101873)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Insert some content.
    pWrtShell->Insert(u"something"_ustr);

    // Search for something which does not exist, twice.
    uno::Sequence<beans::PropertyValue> aFirst(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(u"fig"_ustr) },
        { "SearchItem.Backward", uno::Any(false) },
    }));
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aFirst);
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aFirst);

    uno::Sequence<beans::PropertyValue> aSecond(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(u"something"_ustr) },
        { "SearchItem.Backward", uno::Any(false) },
    }));
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aSecond);

    // Without the accompanying fix in place, this test would have failed with "Expected: something;
    // Actual:", i.e. searching for "something" failed, even if it was inserted above.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(u"something"_ustr, pShellCursor->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTextFormFieldInsertion)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a text form field
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::Fieldmark* pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMTEXT, pFieldmark->GetFieldname());

    // The text form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(vEnSpaces, xPara->getString());

    // Undo insertion
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT(xPara->getString().isEmpty());

    // Redo insertion
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(vEnSpaces, xPara->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testCheckboxFormFieldInsertion)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a checkbox form field
    dispatchCommand(mxComponent, u".uno:CheckBoxFormField"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::Fieldmark* pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMCHECKBOX, pFieldmark->GetFieldname());
    // The checkbox is not checked by default
    ::sw::mark::CheckboxFieldmark* pCheckBox
        = dynamic_cast<::sw::mark::CheckboxFieldmark*>(pFieldmark);
    CPPUNIT_ASSERT(pCheckBox);
    CPPUNIT_ASSERT(!pCheckBox->IsChecked());

    // Undo insertion
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMCHECKBOX, pFieldmark->GetFieldname());

    // tdf#147008 this would crash
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->StartOfSection(false);
    pWrtShell->SplitNode();
    CPPUNIT_ASSERT_EQUAL(pFieldmark->GetMarkPos().GetNodeIndex(),
                         pFieldmark->GetOtherMarkPos().GetNodeIndex());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pFieldmark->GetMarkPos().GetContentIndex() + 1),
                         pFieldmark->GetOtherMarkPos().GetContentIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDropDownFormFieldInsertion)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a drop-down form field
    dispatchCommand(mxComponent, u".uno:DropDownFormField"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::Fieldmark* pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMDROPDOWN, pFieldmark->GetFieldname());
    // Check drop down field's parameters. By default these params are not set
    const sw::mark::Fieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
    auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
    CPPUNIT_ASSERT(bool(pListEntries == pParameters->end()));
    auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
    CPPUNIT_ASSERT(bool(pResult == pParameters->end()));

    // Undo insertion
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMDROPDOWN, pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testMixedFormFieldInsertion)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert fields
    dispatchCommand(mxComponent, u".uno:TextFormField"_ustr, {});
    dispatchCommand(mxComponent, u".uno:CheckBoxFormField"_ustr, {});
    dispatchCommand(mxComponent, u".uno:DropDownFormField"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());

    // Undo insertion
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf147723)
{
    createSwDoc("tdf147723.docx");
    SwDoc* const pDoc = getSwDoc();

    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getAllMarksCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf147006)
{
    createSwDoc("tdf147006.rtf");
    SwDoc* const pDoc = getSwDoc();

    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getFieldmarksCount());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    // this crashed
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDocxAttributeTableExport)
{
    createSwDoc("floating-table-position.docx");

    // get the table frame, set new values and dismiss the references
    {
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);

        // change the properties
        // 8133 -> 8000
        xShape->setPropertyValue(u"VertOrientPosition"_ustr,
                                 uno::Any(static_cast<sal_Int32>(8000)));
        // 5964 -> 5000
        xShape->setPropertyValue(u"HoriOrientPosition"_ustr,
                                 uno::Any(static_cast<sal_Int32>(5000)));
        // 0 (frame) -> 8 (page print area)
        xShape->setPropertyValue(u"VertOrientRelation"_ustr, uno::Any(static_cast<sal_Int16>(8)));
        // 8 (page print area) -> 0 (frame)
        xShape->setPropertyValue(u"HoriOrientRelation"_ustr, uno::Any(static_cast<sal_Int16>(0)));
    }
    // save it to docx
    saveAndReload(u"Office Open XML Text"_ustr);

    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);

    // test the new values
    sal_Int32 nValue = getProperty<sal_Int32>(xShape, u"VertOrientPosition"_ustr);
    CPPUNIT_ASSERT(sal_Int32(7999) <= nValue);
    CPPUNIT_ASSERT(nValue <= sal_Int32(8001));
    nValue = getProperty<sal_Int32>(xShape, u"HoriOrientPosition"_ustr);
    CPPUNIT_ASSERT(sal_Int32(4999) <= nValue);
    CPPUNIT_ASSERT(nValue <= sal_Int32(5001));

    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xShape, u"VertOrientRelation"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xShape, u"HoriOrientRelation"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf118699_redline_numbering)
{
    createSwDoc("tdf118699.docx");
    SwDoc* pDoc = getSwDoc();

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    uno::Reference<beans::XPropertySet> xProps(getParagraph(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: erroneous numbering",
                           !xProps->getPropertyValue(u"NumberingRules"_ustr).hasValue());

    CPPUNIT_ASSERT_MESSAGE("first paragraph after the second deletion: missing numbering",
                           getProperty<uno::Reference<container::XIndexAccess>>(
                               getParagraph(5), u"NumberingRules"_ustr)
                               .is());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf125881_redline_list_level)
{
    createSwDoc("tdf125881.docx");

    uno::Reference<beans::XPropertySet> xProps(getParagraph(8), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("deleted paragraph: erroneous numbering",
                           !xProps->getPropertyValue(u"NumberingRules"_ustr).hasValue());

    // deleted paragraph gets the numbering of the next paragraph
    uno::Reference<beans::XPropertySet> xProps2(getParagraph(9), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: missing numbering",
                           xProps2->getPropertyValue(u"NumberingRules"_ustr).hasValue());

    // check numbering level at deletion (1 instead of 0)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1),
                         getProperty<sal_Int16>(getParagraph(9), u"NumberingLevel"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf125916_redline_restart_numbering)
{
    createSwDoc("tdf125916.docx");
    SwDoc* pDoc = getSwDoc();

    // moveFrom/moveTo are imported as separated redlines from fixing tdf#145718.
    // Accept the first inline moveFrom redline before accepting the remaining ones
    // to leave a paragraph long deletion to test the fix for tdf#125916.
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT(pEditShell->GetRedlineCount() > 0);
    pEditShell->AcceptRedline(0);

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // check unnecessary numbering
    uno::Reference<beans::XPropertySet> xProps(getParagraph(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: erroneous numbering",
                           !xProps->getPropertyValue(u"NumberingRules"_ustr).hasValue());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf125310)
{
    createSwDoc("tdf125310.fodt");
    SwDoc* pDoc = getSwDoc();

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // paragraph join
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    // copied paragraph style
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));

    // without copying the page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf125310b)
{
    createSwDoc("tdf125310b.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // remove second paragraph with the page break
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Down(/*bSelect=*/false);
    pWrtShell->Up(/*bSelect=*/true);
    pWrtShell->DelLeft();

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // losing the page break, as without redlining
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf120336)
{
    createSwDoc("tdf120336.docx");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    CPPUNIT_ASSERT_EQUAL(2, getPages());

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // keep page break, as without redlining
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf106843)
{
    createSwDoc("tdf106843.docx");
    SwDoc* pDoc = getSwDoc();

    // try to turn off red-lining
    dispatchCommand(mxComponent, u".uno:TrackChanges"_ustr, {});

    // but the protection doesn't allow it
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testImageComment)
{
    // Load a document with an as-char image in it.
    createSwDoc("image-comment.odt");
    SwDoc* pDoc = getSwDoc();
    SwView* pView = getSwDocShell()->GetView();

    // Test document has "before<image>after", remove the content before the image.
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // Select the image.
    selectShape(1);

    // Insert a comment while the image is selected.
    pView->GetViewFrame().GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // Verify that the comment is around the image.
    // Without the accompanying fix in place, this test would have failed, as FN_POSTIT was disabled
    // in the frame shell.
    // Then this test would have failed, as in case the as-char anchored image was at the start of
    // the paragraph, the comment of the image covered the character after the image, not the image.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr,
                         getProperty<OUString>(getRun(xPara, 1), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr,
                         getProperty<OUString>(getRun(xPara, 2), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr,
                         getProperty<OUString>(getRun(xPara, 3), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 4), u"TextPortionType"_ustr));

    // Insert content to the comment, and select the image again.
    SfxStringItem aItem(FN_INSERT_STRING, u"x"_ustr);
    pView->GetViewFrame().GetDispatcher()->ExecuteList(FN_INSERT_STRING, SfxCallMode::SYNCHRON,
                                                       { &aItem });
    selectShape(1);

#if !defined(MACOSX)
    // Calc the left edge of the as-char frame.
    SwRootFrame* pLayout = pWrtShell->GetLayout();
    SwFrame* pPage = pLayout->GetLower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pTextFrame = pBody->GetLower();
    CPPUNIT_ASSERT(pTextFrame->GetDrawObjs());
    const SwSortedObjs& rAnchored = *pTextFrame->GetDrawObjs();
    CPPUNIT_ASSERT_GREATER(static_cast<size_t>(0), rAnchored.size());
    SwAnchoredObject* pObject = rAnchored[0];
    tools::Long nFrameLeft = pObject->GetObjRect().Left();
    tools::Long nFrameTop = pObject->GetObjRect().Top();

    // Make sure that the anchor points to the bottom left corner of the image.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected less or equal than: 1418
    // - Actual: 2442
    // The anchor pointed to the bottom right corner, so as-char and at-char was inconsistent.
    Scheduler::ProcessEventsToIdle();
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    for (const auto& pItem : *pPostItMgr)
    {
        const SwRect& rAnchor = pItem->mpPostIt->GetAnchorRect();
        CPPUNIT_ASSERT_EQUAL(nFrameLeft, rAnchor.Left());
    }

    // Test the comment anchor we expose via the LOK API.
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1418, 1418, 0, 0
    // - Actual  : 1418, 1418, 1024, 1024
    // I.e. the anchor position had a non-empty size, which meant different rendering via tiled
    // rendering and on the desktop.
    tools::JsonWriter aJsonWriter;
    SwXTextDocument* pTextDoc = getSwTextDoc();
    pTextDoc->getPostIts(aJsonWriter);
    OString pChar = aJsonWriter.finishAndGetAsOString();
    std::stringstream aStream((std::string(pChar)));
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
    {
        const boost::property_tree::ptree& rComment = rValue.second;
        OString aAnchorPos(rComment.get<std::string>("anchorPos"));
        OString aExpected
            = OString::number(nFrameLeft) + ", " + OString::number(nFrameTop) + ", 0, 0";
        CPPUNIT_ASSERT_EQUAL(aExpected, aAnchorPos);
    }

#endif

    // Now delete the image.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the comment of the image was not deleted when the image was deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testImageCommentAtChar)
{
    // Load a document with an at-char image in it.
    createSwDoc("image-comment-at-char.odt");
    SwDoc* pDoc = getSwDoc();
    SwView* pView = getSwDocShell()->GetView();

    // Select the image.
    selectShape(1);

    // Insert a comment while the image is selected.
    pView->GetViewFrame().GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // Verify that the comment is around the image.
    // Without the accompanying fix in place, this test would have failed, as the comment was
    // anchored at the end of the paragraph, it was not around the image.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 1), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr,
                         getProperty<OUString>(getRun(xPara, 2), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr,
                         getProperty<OUString>(getRun(xPara, 3), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr,
                         getProperty<OUString>(getRun(xPara, 4), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 5), u"TextPortionType"_ustr));

    // Insert content to the comment, and select the image again.
    SfxStringItem aItem(FN_INSERT_STRING, u"x"_ustr);
    pView->GetViewFrame().GetDispatcher()->ExecuteList(FN_INSERT_STRING, SfxCallMode::SYNCHRON,
                                                       { &aItem });
    selectShape(1);
    // Now delete the image.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the comment of the image was not deleted when the image was deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());

    // Undo the deletion and move the image down, so the anchor changes.
    pView->GetViewFrame().GetDispatcher()->Execute(SID_UNDO, SfxCallMode::SYNCHRON);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    Point aNewAnchor = pWrtShell->GetFlyRect().TopLeft();
    aNewAnchor.Move(0, 600);
    pWrtShell->SetFlyPos(aNewAnchor);

    // Get the image anchor doc model position.
    SwFlyFrame* pFly = pWrtShell->GetCurrFlyFrame(false);
    CPPUNIT_ASSERT(pFly);
    SwFrameFormat* pFlyFormat = pFly->GetFrameFormat();
    const SwPosition* pImageAnchor = pFlyFormat->GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT(pImageAnchor);

    // Get the annotation mark doc model start.
    auto it = pDoc->getIDocumentMarkAccess()->getAnnotationMarksBegin();
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAnnotationMarksEnd());
    const sw::mark::MarkBase* pMark = *it;
    const SwPosition& rAnnotationMarkStart = pMark->GetMarkPos();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: SwPosition (node 14, offset 15)
    // - Actual  : SwPosition (node 12, offset 3)
    // This means moving the image anchor did not move the comment anchor / annotation mark, so the
    // image and its comment got out of sync.
    CPPUNIT_ASSERT_EQUAL(*pImageAnchor, rAnnotationMarkStart);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTrackImageDeletion)
{
    // load a document with an image anchored to paragraph in it
    createSwDoc("image.odt");
    SwDoc* pDoc = getSwDoc();
    SwView* pView = getSwDocShell()->GetView();

    // select the image
    selectShape(1);

    // turn on red-lining and show changes
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());

    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // now delete the image with track changes
    pView->GetViewFrame().GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);

    const SwRedlineTable& rTable = rIDRA.GetRedlineTable();
    // this was 0 (missing recording of deletion of images)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());

    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // tdf#142701 this was AS_CHARACTER (convert AT_PARA to AT_CHAR to keep the layout)
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xShape, u"AnchorType"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTrackImageInsertion)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());

    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Insert an image with change tracking
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FileName"_ustr, createFileURL(u"ole2.png")),
    };
    dispatchCommand(mxComponent, u".uno:InsertGraphic"_ustr, aArgs);

    const SwRedlineTable& rTable = rIDRA.GetRedlineTable();
    // this was 0 (missing recording of insertion of images)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf120338)
{
    createSwDoc("tdf120338.docx");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaAdjust"_ustr)); // right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(3), u"ParaAdjust"_ustr)); // right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(4), u"ParaAdjust"_ustr)); // left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(5), u"ParaAdjust"_ustr)); // right

    CPPUNIT_ASSERT_EQUAL(u""_ustr,
                         getProperty<OUString>(getParagraph(7), u"NumberingStyleName"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"WWNum2"_ustr,
                         getProperty<OUString>(getParagraph(8), u"NumberingStyleName"_ustr));

    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(10), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(11), u"ParaStyleName"_ustr));

    // reject tracked paragraph adjustments
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaAdjust"_ustr)); // left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(3), u"ParaAdjust"_ustr)); // center
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(4), u"ParaAdjust"_ustr)); // center
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(5), u"ParaAdjust"_ustr)); // left

    // tdf#126245 revert numbering changes
    CPPUNIT_ASSERT_EQUAL(u"WWNum2"_ustr,
                         getProperty<OUString>(getParagraph(7), u"NumberingStyleName"_ustr));

    CPPUNIT_ASSERT_EQUAL(u""_ustr,
                         getProperty<OUString>(getParagraph(8), u"NumberingStyleName"_ustr));

    // tdf#126243 revert paragraph styles
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(10), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 3"_ustr,
                         getProperty<OUString>(getParagraph(11), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf120338_multiple_paragraph_join)
{
    createSwDoc("redline-para-join.docx");

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));

    // reject tracked paragraph styles
    dispatchCommand(mxComponent, u".uno:RejectAllTrackedChanges"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                         getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 2"_ustr,
                         getProperty<OUString>(getParagraph(2), u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Heading 3"_ustr,
                         getProperty<OUString>(getParagraph(3), u"ParaStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testShapePageMove)
{
    // Load a document with 2 pages, shape on the first page.
    createSwDoc("shape-page-move.odt");
    SwDoc* pDoc = getSwDoc();
    SwView* pView = getSwDocShell()->GetView();
    // Make sure that the 2nd page is below the 1st one.
    pView->SetViewLayout(/*nColumns=*/1, /*bBookMode=*/false);
    calcLayout();

    // Select the shape.
    selectShape(1);

    // Move the shape down to the 2nd page.
    SfxInt32Item aXItem(SID_ATTR_TRANSFORM_POS_X, 4000);
    SfxInt32Item aYItem(SID_ATTR_TRANSFORM_POS_Y, 12000);
    pView->GetViewFrame().GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM, SfxCallMode::SYNCHRON,
                                                       { &aXItem, &aYItem });

    // Check if the shape anchor was moved to the 2nd page as well.
    auto pShapeFormats = pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(!pShapeFormats->empty());
    auto it = pShapeFormats->begin();
    auto pShapeFormat = *it;
    const SwPosition* pAnchor = pShapeFormat->GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT(pAnchor);

    // Find out the node index of the 1st para on the 2nd page.
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pFirstPage = pLayout->Lower();
    SwFrame* pSecondPage = pFirstPage->GetNext();
    CPPUNIT_ASSERT(pSecondPage->IsLayoutFrame());
    SwFrame* pBodyFrame = static_cast<SwLayoutFrame*>(pSecondPage)->GetLower();
    CPPUNIT_ASSERT(pBodyFrame->IsLayoutFrame());
    SwFrame* pTextFrame = static_cast<SwLayoutFrame*>(pBodyFrame)->GetLower();
    CPPUNIT_ASSERT(pTextFrame->IsTextFrame());
    SwNodeOffset nNodeIndex = static_cast<SwTextFrame*>(pTextFrame)->GetTextNodeFirst()->GetIndex();

    // Without the accompanying fix in place, this test would have failed with "Expected: 13;
    // Actual: 12", i.e. the shape was anchored to the last paragraph of the 1st page, not to a
    // paragraph on the 2nd page.
    CPPUNIT_ASSERT_EQUAL(nNodeIndex, pAnchor->GetNodeIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDateFormFieldInsertion)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, u".uno:DatePickerFormField"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::Fieldmark* pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMDATE, pFieldmark->GetFieldname());

    // The date form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(vEnSpaces, xPara->getString());

    // Undo insertion
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, u".uno:Redo"_ustr, {});
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::Fieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMDATE, pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDateFormFieldContentOperations)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, u".uno:DatePickerFormField"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::DateFieldmark* pFieldmark = dynamic_cast<::sw::mark::DateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMDATE, pFieldmark->GetFieldname());

    // Check the default content added by insertion
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(vEnSpaces, pFieldmark->GetContent());

    // Set content to empty string
    pFieldmark->ReplaceContent(u""_ustr);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, pFieldmark->GetContent());

    // Replace empty string with a valid content
    pFieldmark->ReplaceContent(u"2019-10-23"_ustr);
    CPPUNIT_ASSERT_EQUAL(u"2019-10-23"_ustr, pFieldmark->GetContent());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDateFormFieldCurrentDateHandling)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, u".uno:DatePickerFormField"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::DateFieldmark* pFieldmark = dynamic_cast<::sw::mark::DateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMDATE, pFieldmark->GetFieldname());

    // The default content is not a valid date
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(vEnSpaces, pFieldmark->GetContent());
    std::pair<bool, double> aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(!aResult.first);

    // Check empty string
    pFieldmark->ReplaceContent(u""_ustr);
    aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(!aResult.first);

    // Check valid date
    // Set date format first
    sw::mark::Fieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
    (*pParameters)[ODF_FORMDATE_DATEFORMAT] <<= u"YYYY/MM/DD"_ustr;
    (*pParameters)[ODF_FORMDATE_DATEFORMAT_LANGUAGE] <<= u"en-US"_ustr;

    // Set date value and check whether the content is formatted correctly
    pFieldmark->SetCurrentDate(48000.0);
    aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(aResult.first);
    CPPUNIT_ASSERT_EQUAL(48000.0, aResult.second);
    CPPUNIT_ASSERT_EQUAL(u"2031/06/01"_ustr, pFieldmark->GetContent());
    // Current date param contains date in a "standard format"
    OUString sCurrentDate;
    auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    CPPUNIT_ASSERT_EQUAL(u"2031-06-01"_ustr, sCurrentDate);
}

#if !defined(_WIN32)
CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDateFormFieldCurrentDateInvalidation)
{
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, u".uno:DatePickerFormField"_ustr, {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::DateFieldmark* pFieldmark = dynamic_cast<::sw::mark::DateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(ODF_FORMDATE, pFieldmark->GetFieldname());

    // Set a date first
    sw::mark::Fieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
    pFieldmark->SetCurrentDate(48000.0);
    std::pair<bool, double> aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(aResult.first);
    CPPUNIT_ASSERT_EQUAL(48000.0, aResult.second);

    // Do the layouting to trigger invalidation
    // Since we have the current date consistent with the field content
    // This invalidation won't change anything
    calcLayout();
    Scheduler::ProcessEventsToIdle();

    // Current date param contains date in a "standard format"
    OUString sCurrentDate;
    auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    // We have the current date parameter set
    CPPUNIT_ASSERT_EQUAL(u"2031-06-01"_ustr, sCurrentDate);

    // Now change the content of the field
    pFieldmark->ReplaceContent(u"[select date]"_ustr);
    // Do the layouting to trigger invalidation
    calcLayout();
    Scheduler::ProcessEventsToIdle();

    sCurrentDate.clear();
    pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    CPPUNIT_ASSERT_EQUAL(u""_ustr, sCurrentDate);
}
#endif

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testOleSaveWhileEdit)
{
    // Enable LOK mode, otherwise OCommonEmbeddedObject::SwitchStateTo_Impl() will throw when it
    // finds out that the test runs headless.
    comphelper::LibreOfficeKit::setActive();

    // Load a document with a Draw doc in it.
    createSwDoc("ole-save-while-edit.odt");
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();

    selectShape(1);

    // Start editing the OLE object.
    pWrtShell->LaunchOLEObj();

    // Save the document without existing the OLE edit.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), {});

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      maTempFile.GetURL());
    // Without the accompanying fix in place, this test would have failed: the OLE object lost its
    // replacement on save if the edit was active while saving.
    CPPUNIT_ASSERT(xNameAccess->hasByName(u"ObjectReplacements/Object 1"_ustr));

    // Dispose the document while LOK is still active to avoid leaks.
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf105330)
{
    createSwDoc("tdf105330.odt");
    SwDoc* pDoc = getSwDoc();

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwView* pView = getSwDocShell()->GetView();
    SfxUInt16Item aRows(SID_ATTR_TABLE_ROW, 1);
    SfxUInt16Item aColumns(SID_ATTR_TABLE_COLUMN, 1);
    pView->GetViewFrame().GetDispatcher()->ExecuteList(FN_INSERT_TABLE, SfxCallMode::SYNCHRON,
                                                       { &aRows, &aColumns });

    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();

    // Without the accompanying fix in place, height was only 1 twips (practically invisible).
    // Require at least 12pt height (font size under the cursor), in twips.
    CPPUNIT_ASSERT_GREATEREQUAL(
        static_cast<tools::Long>(240),
        pWrtShell->GetVisibleCursor()->GetTextCursor().GetSize().getHeight());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf118311)
{
    createSwDoc("tdf118311.fodt");

    // Jump to the first cell, selecting its content
    uno::Sequence<beans::PropertyValue> aSearch(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(u"a"_ustr) },
        { "SearchItem.Backward", uno::Any(false) },
    }));
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aSearch);

    //  .uno:Cut doesn't remove the table, only the selected content of the first cell
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // .uno:SelectAll selects the whole table, and UNO command Cut cuts it
    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Cut"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowDeletion)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    createSwDoc("tdf118311.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    // (HasTextChangesOnly property of the row will be false)
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    // This was deleted without change tracking
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);

    // Undo, and repeat the previous test, but only with deletion of the text content of the cells
    // (HasTextChangesOnly property will be removed by Undo)

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // table exists again
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    dispatchCommand(mxComponent, u".uno:SelectRow"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Delete"_ustr, {});

    // Table row still exists
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept the deletion of the content of the first cell
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) still exists
    // (HasTextChangesOnly property wasn't set for table row deletion)
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Undo, and delete the row without change tracking

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // table exists again
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // disable change tracking
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);

    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // delete table row without change tracking
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    // the table (row) was deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf150976)
{
    // load a 1-row table, and delete the row with track changes
    createSwDoc("select-row.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    // nested table in the last cell
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell[2]/tab");

    // delete table row with enabled change tracking
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // deleted text content
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    // This was 1 before fixing tdf#151478 (testSelectRowWithNestedTable)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(3), pEditShell->GetRedlineCount());

    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->Lower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pTable = pBody->GetLower();
    CPPUNIT_ASSERT(pTable->IsTabFrame());

    SwTabFrame* pTabFrame = static_cast<SwTabFrame*>(pTable);

    // This was false (not deleted row)
    CPPUNIT_ASSERT(pTabFrame->GetTable()->HasDeletedRowOrCell());

    // accept all tracked changes
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

    pXmlDoc = parseLayoutDump();

    // tdf#151658 This was 1: not deleted table row (and table)
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf151657)
{
    // load a 1-row table, and delete the row with hidden track changes
    createSwDoc("select-row.fodt");
    SwDoc* pDoc = getSwDoc();

    // turn on red-lining and hide changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines should be invisible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    // nested table in the last cell
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell[2]/tab");

    // delete table row with enabled change tracking
    dispatchCommand(mxComponent, u".uno:DeleteRows"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // deleted text content
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    // This was 1 before fixing tdf#151478 (testSelectRowWithNestedTable)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(3), pEditShell->GetRedlineCount());

    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->Lower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pTable = pBody->GetLower();
    CPPUNIT_ASSERT(pTable->IsTabFrame());

    SwTabFrame* pTabFrame = static_cast<SwTabFrame*>(pTable);

    // This was false (not deleted row)
    CPPUNIT_ASSERT(pTabFrame->GetTable()->HasDeletedRowOrCell());

    // accept all tracked changes
    dispatchCommand(mxComponent, u".uno:AcceptAllTrackedChanges"_ustr, {});

    pXmlDoc = parseLayoutDump();

    // tdf#151658 This was 1: not deleted table row (and table)
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testSelectRowWithNestedTable)
{
    // load a 1-row table, and select the row
    createSwDoc("select-row.fodt");

    // check table
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    // nested table in the last cell
    assertXPath(pXmlDoc, "//page[1]//body/tab/row/cell[2]/tab");

    // select table row
    dispatchCommand(mxComponent, u".uno:EntireRow"_ustr, {});
    // convert selected text content to uppercase
    dispatchCommand(mxComponent, u".uno:ChangeCaseToUpper"_ustr, {});

    pXmlDoc = parseLayoutDump();
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row/cell[2]/tab/row/cell[1]/txt",
                       u"NESTED-A1");
    // This was "a1" (bad selection of the table row)
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row/cell[1]/txt[1]", u"A1");
    // This was "nested-b1" (bad selection of the table row)
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row/cell[2]/tab/row/cell[2]/txt",
                       u"NESTED-B1");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf47979_row)
{
    // load a 2-row table, and select row 2 by clicking before it
    createSwDoc("select-column.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->Lower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pTable = pBody->GetLower()->GetNext();
    SwFrame* pRow2 = pTable->GetLower()->GetNext();
    const SwRect& rRow2Rect = pRow2->getFrameArea();
    Point ptRow(rRow2Rect.Left(), rRow2Rect.Top() + rRow2Rect.Height() / 2);

    pWrtShell->SelectTableRowCol(ptRow);

    // convert selected text content to uppercase
    dispatchCommand(mxComponent, u".uno:ChangeCaseToUpper"_ustr, {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[1]/cell[1]/txt[1]", u"a1");
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[1]/cell[2]/txt[1]", u"b1");
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[2]/cell[2]/tab/row/cell[1]/txt",
                       u"NESTED-A1");
    // This was "a2" (bad selection of the table row)
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[2]/cell[1]/txt[1]", u"A2");
    // This was "nested-b1" (bad selection of the table row)
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[2]/cell[2]/tab/row/cell[2]/txt",
                       u"NESTED-B1");
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf47979_column)
{
    // load a 2-row table, and select column B by clicking before them
    createSwDoc("select-column.fodt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // select table column by using the middle point of the top border of column B
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwFrame* pPage = pLayout->Lower();
    SwFrame* pBody = pPage->GetLower();
    SwFrame* pTable = pBody->GetLower()->GetNext();
    SwFrame* pRow1 = pTable->GetLower();
    SwFrame* pCellB1 = pRow1->GetLower()->GetNext();
    const SwRect& rCellB1Rect = pCellB1->getFrameArea();
    Point ptColumn(rCellB1Rect.Left() + rCellB1Rect.Width() / 2, rCellB1Rect.Top() - 5);

    pWrtShell->SelectTableRowCol(ptColumn);

    // convert selected text content to uppercase
    dispatchCommand(mxComponent, u".uno:ChangeCaseToUpper"_ustr, {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[1]/cell[1]/txt[1]", u"a1");
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[2]/cell[1]/txt[1]", u"a2");
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[2]/cell[2]/tab/row/cell[1]/txt",
                       u"NESTED-A1");
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[2]/cell[2]/tab/row/cell[2]/txt",
                       u"NESTED-B1");
    // This was "b1" (bad selection of the table column)
    assertXPathContent(pXmlDoc, "//page[1]//body/tab/row[1]/cell[2]/txt[1]", u"B1");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
