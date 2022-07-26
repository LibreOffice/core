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
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/configuration.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <vcl/settings.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
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
#include <view.hxx>
#include <cmdid.h>
#include <AnnotationWin.hxx>
#include <PostItMgr.hxx>
#include <fmtcntnt.hxx>
#include <frameformats.hxx>
#include <shellio.hxx>
#include <editeng/fontitem.hxx>
#include <tools/json_writer.hxx>
#include <unotxdoc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/uiwriter/data/";
} // namespace

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest5 : public SwModelTestBase
{
public:
    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        m_aSavedSettings = Application::GetSettings();
        if (OString(filename).indexOf("LocaleArabic") != -1)
        {
            std::unique_ptr<Resetter> pResetter(
                new Resetter([this]() { Application::SetSettings(this->m_aSavedSettings); }));
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("ar"));
            Application::SetSettings(aSettings);
            return pResetter;
        }
        return nullptr;
    }

protected:
    AllSettings m_aSavedSettings;
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf139127)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf139127.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();
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
    dispatchCommand(mxComponent, ".uno:GoToEndOfDoc", {});
    dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    dispatchCommand(mxComponent, ".uno:SwBackspace", {});
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    CPPUNIT_ASSERT_EQUAL(OUString("First page"), pTextDoc->getText()->getString());

    // Undo
    dispatchCommand(mxComponent, ".uno:Undo", {});
    // this would crash due to bad redline range
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    CPPUNIT_ASSERT_EQUAL(OUString("First page"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("B"), getParagraph(2)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf138479)
{
    SwDoc* const pDoc = createSwDoc();

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    pWrtShell->Insert("Lorem");
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), getParagraph(1)->getString());

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
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // delete "r" in "Lorem"
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 3, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Loem"), getParagraph(1)->getString());

    // delete "oe" in "Loem"
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 2, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Lm"), getParagraph(1)->getString());

    // test embedded Undo in ChangesInMargin mode
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Loem"), getParagraph(1)->getString());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), getParagraph(1)->getString());

    // this would crash due to bad redline range
    for (int i = 0; i < 5; ++i)
        dispatchCommand(mxComponent, ".uno:Undo", {});

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});
    CPPUNIT_ASSERT(!pWrtShell->GetViewOptions()->IsShowChangesInMargin());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf138666)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf39721.fodt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // show deletions inline
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("dolor sit"), getParagraph(2)->getString());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // show deletions in margin
    CPPUNIT_ASSERT_EQUAL(OUString("Loremm"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("dolsit"), getParagraph(2)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // show deletions inline again
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("dolor sit"), getParagraph(2)->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf140982)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf115815.odt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // show deletions inline
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum dolor sit amet..."), getParagraph(1)->getString());

    // switch on "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // show deletions in margin
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem  amet..."), getParagraph(1)->getString());

    // switch off "Show changes in margin" mode
    dispatchCommand(mxComponent, ".uno:ShowChangesInMargin", {});

    // show deletions inline again
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum dolor sit amet..."), getParagraph(1)->getString());

    // Save it and load it back.
    reload("writer8", "tdf115815.odt");

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
        OUString aType = getProperty<OUString>(xPropertySet, "TextPortionType");
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
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), sTextBeforeAnnotation);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf126206)
{
    createSwDoc(DATA_DIRECTORY, "tdf126206.docx");

    // normal text (it was bold)
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 4)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 3)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf50447)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126206.docx");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }

    // remove bold formatting with change tracking
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Bold", {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        // This was NORMAL
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143918)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126206.docx");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }

    // remove bold formatting with change tracking and after that, apply underline, too
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    dispatchCommand(mxComponent, ".uno:Bold", {});
    dispatchCommand(mxComponent, ".uno:Underline", {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        // This was NORMAL (only underlining was removed)
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143938)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf54819.fodt");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // select first paragraph, add underline without change tracking
    pWrtShell->EndPara(/*bSelect=*/true);
    dispatchCommand(mxComponent, ".uno:Underline", {});

    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
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
    dispatchCommand(mxComponent, ".uno:Italic", {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        // (first empty run is associated to the redline)
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // no italic, but still underline direct formatting
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum"), xCursor->getString());
        // This wasn't underlined (lost direct formatting)
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143939)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf126206.docx");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // bold text
    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }

    // positionate the text cursor inside the first word
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    // remove bold formatting with change tracking without selection
    dispatchCommand(mxComponent, ".uno:Bold", {});

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 2)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xCursor, "CharWeight"));
    }

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // bold text again
    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), xCursor->getString());
        // This was NORMAL
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(xCursor, "CharWeight"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf50447_keep_hints)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf50447.fodt");

    // first paragraph (_Lorem_ /ipsum/)

    auto xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString(" "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }

    // second paragraph (_dolor_ sit /amet/.)

    xText = getParagraph(2)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(2), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("dolor"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString(" sit "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("amet"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
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

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SuperScript", {});

    // multiple format redlines for the multiple hints

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // This was 1.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(7), pEditShell->GetRedlineCount());

    // reject tracked changes

    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // all hints and text portions between them got back the original formatting

    xText = getParagraph(1)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(1), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("Lorem"), xCursor->getString());
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString(" "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(1), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("ipsum"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }

    // second paragraph (_dolor_ sit /amet/.)

    xText = getParagraph(2)->getText();
    CPPUNIT_ASSERT(xText.is());
    {
        auto xCursor(xText->createTextCursorByRange(getRun(getParagraph(2), 1)));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("dolor"), xCursor->getString());
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 2));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString(" sit "), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_NONE,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));

        xCursor = xText->createTextCursorByRange(getRun(getParagraph(2), 3));
        CPPUNIT_ASSERT(xCursor.is());
        CPPUNIT_ASSERT_EQUAL(OUString("amet"), xCursor->getString());
        CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::NONE),
                             getProperty<sal_Int16>(xCursor, "CharUnderline"));
        // This was NONE
        CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC,
                             getProperty<awt::FontSlant>(xCursor, "CharPosture"));
    }
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf144272)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf50447.fodt");

    SwWrtShell* const pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // paragraph style of the first paragraph: Heading 1, second paragraph: Standard

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // modify with track changes: Standard and Heading 2

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(OUString("Standard")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);
    pWrtShell->Down(/*bSelect=*/false);
    aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(OUString("Heading 2")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // reject tracked changes
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    // This was Standard (missing reject)
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    // This was Heading 2 (missing reject)
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf101873)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    // Insert some content.
    pWrtShell->Insert("something");

    // Search for something which does not exist, twice.
    uno::Sequence<beans::PropertyValue> aFirst(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(OUString("fig")) },
        { "SearchItem.Backward", uno::Any(false) },
    }));
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aFirst);
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aFirst);

    uno::Sequence<beans::PropertyValue> aSecond(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(OUString("something")) },
        { "SearchItem.Backward", uno::Any(false) },
    }));
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aSecond);

    // Without the accompanying fix in place, this test would have failed with "Expected: something;
    // Actual:", i.e. searching for "something" failed, even if it was inserted above.
    SwShellCursor* pShellCursor = pWrtShell->getShellCursor(false);
    CPPUNIT_ASSERT_EQUAL(OUString("something"), pShellCursor->GetText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTextFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a text form field
    dispatchCommand(mxComponent, ".uno:TextFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMTEXT), pFieldmark->GetFieldname());

    // The text form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT(xPara->getString().isEmpty());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    xPara.set(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testCheckboxFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a checkbox form field
    dispatchCommand(mxComponent, ".uno:CheckBoxFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());
    // The checkbox is not checked by default
    ::sw::mark::ICheckboxFieldmark* pCheckBox
        = dynamic_cast<::sw::mark::ICheckboxFieldmark*>(pFieldmark);
    CPPUNIT_ASSERT(pCheckBox);
    CPPUNIT_ASSERT(!pCheckBox->IsChecked());

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMCHECKBOX), pFieldmark->GetFieldname());

    // tdf#147008 this would crash
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->StartOfSection(false);
    pWrtShell->SplitNode();
    CPPUNIT_ASSERT_EQUAL(pFieldmark->GetMarkPos().nNode, pFieldmark->GetOtherMarkPos().nNode);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(pFieldmark->GetMarkPos().nContent.GetIndex() + 1),
                         pFieldmark->GetOtherMarkPos().nContent.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDropDownFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a drop-down form field
    dispatchCommand(mxComponent, ".uno:DropDownFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());
    // Check drop down field's parameters. By default these params are not set
    const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
    auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
    CPPUNIT_ASSERT(bool(pListEntries == pParameters->end()));
    auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
    CPPUNIT_ASSERT(bool(pResult == pParameters->end()));

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDROPDOWN), pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testMixedFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);

    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert fields
    dispatchCommand(mxComponent, ".uno:TextFormField", {});
    dispatchCommand(mxComponent, ".uno:CheckBoxFormField", {});
    dispatchCommand(mxComponent, ".uno:DropDownFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), pMarkAccess->getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf147723)
{
    SwDoc* const pDoc = createSwDoc(DATA_DIRECTORY, "tdf147723.docx");

    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getAllMarksCount());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});

    // Without the fix in place, this test would have crashed here
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, ".uno:Paste", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), rIDMA.getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf147006)
{
    SwDoc* const pDoc = createSwDoc(DATA_DIRECTORY, "tdf147006.rtf");

    IDocumentMarkAccess& rIDMA(*pDoc->getIDocumentMarkAccess());
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(std::iterator_traits<IDocumentMarkAccess::iterator>::difference_type(1),
                         std::distance(rIDMA.getFieldmarksBegin(), rIDMA.getFieldmarksEnd()));

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    // this crashed
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), rIDMA.getAllMarksCount());
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_FRM));
    CPPUNIT_ASSERT_EQUAL(size_t(0), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rIDMA.getAllMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDocxAttributeTableExport)
{
    createSwDoc(DATA_DIRECTORY, "floating-table-position.docx");

    // get the table frame, set new values and dismiss the references
    {
        uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);

        // change the properties
        // 8133 -> 8000
        xShape->setPropertyValue("VertOrientPosition", uno::Any(static_cast<sal_Int32>(8000)));
        // 5964 -> 5000
        xShape->setPropertyValue("HoriOrientPosition", uno::Any(static_cast<sal_Int32>(5000)));
        // 0 (frame) -> 8 (page print area)
        xShape->setPropertyValue("VertOrientRelation", uno::Any(static_cast<sal_Int16>(8)));
        // 8 (page print area) -> 0 (frame)
        xShape->setPropertyValue("HoriOrientRelation", uno::Any(static_cast<sal_Int16>(0)));
    }
    // save it to docx
    reload("Office Open XML Text", "floating-table-position.docx");

    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);

    // test the new values
    sal_Int32 nValue = getProperty<sal_Int32>(xShape, "VertOrientPosition");
    CPPUNIT_ASSERT(sal_Int32(7999) <= nValue);
    CPPUNIT_ASSERT(nValue <= sal_Int32(8001));
    nValue = getProperty<sal_Int32>(xShape, "HoriOrientPosition");
    CPPUNIT_ASSERT(sal_Int32(4999) <= nValue);
    CPPUNIT_ASSERT(nValue <= sal_Int32(5001));

    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xShape, "VertOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), getProperty<sal_Int16>(xShape, "HoriOrientRelation"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf118699_redline_numbering)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118699.docx");

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    uno::Reference<beans::XPropertySet> xProps(getParagraph(2), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());

    CPPUNIT_ASSERT_MESSAGE(
        "first paragraph after the second deletion: missing numbering",
        getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(5), "NumberingRules")
            .is());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf125881_redline_list_level)
{
    createSwDoc(DATA_DIRECTORY, "tdf125881.docx");

    uno::Reference<beans::XPropertySet> xProps(getParagraph(8), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("deleted paragraph: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());

    // deleted paragraph gets the numbering of the next paragraph
    uno::Reference<beans::XPropertySet> xProps2(getParagraph(9), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: missing numbering",
                           xProps2->getPropertyValue("NumberingRules").hasValue());

    // check numbering level at deletion (1 instead of 0)
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(getParagraph(9), "NumberingLevel"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf125916_redline_restart_numbering)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125916.docx");

    // moveFrom/moveTo are imported as separated redlines from fixing tdf#145718.
    // Accept the first inline moveFrom redline before accepting the remaining ones
    // to leave a paragraph long deletion to test the fix for tdf#125916.
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT(pEditShell->GetRedlineCount() > 0);
    pEditShell->AcceptRedline(0);

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // check unnecessary numbering
    uno::Reference<beans::XPropertySet> xProps(getParagraph(3), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("first paragraph after the first deletion: erroneous numbering",
                           !xProps->getPropertyValue("NumberingRules").hasValue());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf125310)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125310.fodt");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
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
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->EndPara(/*bSelect=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // copied paragraph style
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));

    // without copying the page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf125310b)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf125310b.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf120336.docx");

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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf106843.docx");

    // try to turn off red-lining
    dispatchCommand(mxComponent, ".uno:TrackChanges", {});

    // but the protection doesn't allow it
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testImageComment)
{
    // Load a document with an as-char image in it.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "image-comment.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();

    // Test document has "before<image>after", remove the content before the image.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStart=*/true);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 6, /*bBasicCall=*/false);
    pWrtShell->Delete();

    // Select the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

    // Insert a comment while the image is selected.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // Verify that the comment is around the image.
    // Without the accompanying fix in place, this test would have failed, as FN_POSTIT was disabled
    // in the frame shell.
    // Then this test would have failed, as in case the as-char anchored image was at the start of
    // the paragraph, the comment of the image covered the character after the image, not the image.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));

    // Insert content to the comment, and select the image again.
    SfxStringItem aItem(FN_INSERT_STRING, "x");
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_STRING, SfxCallMode::SYNCHRON,
                                                        { &aItem });
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

#if !defined(MACOSX)
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
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
    pTextDoc->getPostIts(aJsonWriter);
    char* pChar = aJsonWriter.extractData();
    std::stringstream aStream(pChar);
    free(pChar);
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    for (const boost::property_tree::ptree::value_type& rValue : aTree.get_child("comments"))
    {
        const boost::property_tree::ptree& rComment = rValue.second;
        OString aAnchorPos(rComment.get<std::string>("anchorPos").c_str());
        OString aExpected
            = OString::number(nFrameLeft) + ", " + OString::number(nFrameTop) + ", 0, 0";
        CPPUNIT_ASSERT_EQUAL(aExpected, aAnchorPos);
    }

#endif

    // Now delete the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the comment of the image was not deleted when the image was deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testImageCommentAtChar)
{
    // Load a document with an at-char image in it.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "image-comment-at-char.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();

    // Select the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

    // Insert a comment while the image is selected.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_POSTIT, SfxCallMode::SYNCHRON);

    // Verify that the comment is around the image.
    // Without the accompanying fix in place, this test would have failed, as the comment was
    // anchored at the end of the paragraph, it was not around the image.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 5), "TextPortionType"));

    // Insert content to the comment, and select the image again.
    SfxStringItem aItem(FN_INSERT_STRING, "x");
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_STRING, SfxCallMode::SYNCHRON,
                                                        { &aItem });
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Now delete the image.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);
    // Without the accompanying fix in place, this test would have failed with 'Expected: 0; Actual:
    // 1', i.e. the comment of the image was not deleted when the image was deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());

    // Undo the deletion and move the image down, so the anchor changes.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_UNDO, SfxCallMode::SYNCHRON);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         pDoc->getIDocumentMarkAccess()->getAnnotationMarksCount());
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    Point aNewAnchor = pWrtShell->GetFlyRect().TopLeft();
    aNewAnchor.Move(0, 600);
    pWrtShell->SetFlyPos(aNewAnchor);

    // Get the image anchor doc model position.
    SwFlyFrame* pFly = pWrtShell->GetCurrFlyFrame(false);
    CPPUNIT_ASSERT(pFly);
    SwFrameFormat& rFlyFormat = pFly->GetFrameFormat();
    const SwPosition* pImageAnchor = rFlyFormat.GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT(pImageAnchor);

    // Get the annotation mark doc model start.
    auto it = pDoc->getIDocumentMarkAccess()->getAnnotationMarksBegin();
    CPPUNIT_ASSERT(it != pDoc->getIDocumentMarkAccess()->getAnnotationMarksEnd());
    const sw::mark::IMark* pMark = *it;
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
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "image.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();

    // select the image
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);

    // turn on red-lining and show changes
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());

    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // now delete the image with track changes
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_DELETE, SfxCallMode::SYNCHRON);

    const SwRedlineTable& rTable = rIDRA.GetRedlineTable();
    // this was 0 (missing recording of deletion of images)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());

    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // tdf#142701 this was AS_CHARACTER (convert AT_PARA to AT_CHAR to keep the layout)
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xShape, "AnchorType"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTrackImageInsertion)
{
    SwDoc* pDoc = createSwDoc();

    // turn on red-lining and show changes
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());

    rIDRA.SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // Insert an image with change tracking
    OUString aImageURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "ole2.png";
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("FileName", aImageURL),
    };
    dispatchCommand(mxComponent, ".uno:InsertGraphic", aArgs);

    const SwRedlineTable& rTable = rIDRA.GetRedlineTable();
    // this was 0 (missing recording of insertion of images)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), rTable.size());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf120338)
{
    createSwDoc(DATA_DIRECTORY, "tdf120338.docx");

    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // right
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(4), "ParaAdjust")); // left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         getProperty<sal_Int32>(getParagraph(5), "ParaAdjust")); // right

    CPPUNIT_ASSERT_EQUAL(OUString(""),
                         getProperty<OUString>(getParagraph(7), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString("WWNum2"),
                         getProperty<OUString>(getParagraph(8), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(10), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(11), "ParaStyleName"));

    // reject tracked paragraph adjustments
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(2), "ParaAdjust")); // left
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(3), "ParaAdjust")); // center
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3),
                         getProperty<sal_Int32>(getParagraph(4), "ParaAdjust")); // center
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(getParagraph(5), "ParaAdjust")); // left

    // tdf#126245 revert numbering changes
    CPPUNIT_ASSERT_EQUAL(OUString("WWNum2"),
                         getProperty<OUString>(getParagraph(7), "NumberingStyleName"));

    CPPUNIT_ASSERT_EQUAL(OUString(""),
                         getProperty<OUString>(getParagraph(8), "NumberingStyleName"));

    // tdf#126243 revert paragraph styles
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         getProperty<OUString>(getParagraph(10), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 3"),
                         getProperty<OUString>(getParagraph(11), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf120338_multiple_paragraph_join)
{
    createSwDoc(DATA_DIRECTORY, "redline-para-join.docx");

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));

    // reject tracked paragraph styles
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 2"),
                         getProperty<OUString>(getParagraph(2), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 3"),
                         getProperty<OUString>(getParagraph(3), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testShapePageMove)
{
    // Load a document with 2 pages, shape on the first page.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "shape-page-move.odt");
    SwView* pView = pDoc->GetDocShell()->GetView();
    // Make sure that the 2nd page is below the 1st one.
    pView->SetViewLayout(/*nColumns=*/1, /*bBookMode=*/false);
    calcLayout();

    // Select the shape.
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();

    // Move the shape down to the 2nd page.
    SfxInt32Item aXItem(SID_ATTR_TRANSFORM_POS_X, 4000);
    SfxInt32Item aYItem(SID_ATTR_TRANSFORM_POS_Y, 12000);
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM, SfxCallMode::SYNCHRON,
                                                        { &aXItem, &aYItem });

    // Check if the shape anchor was moved to the 2nd page as well.
    SwFrameFormats* pShapeFormats = pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(!pShapeFormats->empty());
    auto it = pShapeFormats->begin();
    SwFrameFormat* pShapeFormat = *it;
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
    CPPUNIT_ASSERT_EQUAL(nNodeIndex, pAnchor->nNode.GetIndex());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDateFormFieldInsertion)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // The date form field has the placeholder text in it
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), xPara->getString());

    // Undo insertion
    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Redo insertion
    dispatchCommand(mxComponent, ".uno:Redo", {});
    aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDateFormFieldContentOperations)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // Check the default content added by insertion
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), pFieldmark->GetContent());

    // Set content to empty string
    pFieldmark->ReplaceContent("");
    CPPUNIT_ASSERT_EQUAL(OUString(""), pFieldmark->GetContent());

    // Replace empty string with a valid content
    pFieldmark->ReplaceContent("2019-10-23");
    CPPUNIT_ASSERT_EQUAL(OUString("2019-10-23"), pFieldmark->GetContent());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDateFormFieldCurrentDateHandling)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // The default content is not a valid date
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    sal_Unicode vEnSpaces[5] = { 8194, 8194, 8194, 8194, 8194 };
    CPPUNIT_ASSERT_EQUAL(OUString(vEnSpaces, 5), pFieldmark->GetContent());
    std::pair<bool, double> aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(!aResult.first);

    // Check empty string
    pFieldmark->ReplaceContent("");
    aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(!aResult.first);

    // Check valid date
    // Set date format first
    sw::mark::IFieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
    (*pParameters)[ODF_FORMDATE_DATEFORMAT] <<= OUString("YYYY/MM/DD");
    (*pParameters)[ODF_FORMDATE_DATEFORMAT_LANGUAGE] <<= OUString("en-US");

    // Set date value and check whether the content is formatted correctly
    pFieldmark->SetCurrentDate(48000.0);
    aResult = pFieldmark->GetCurrentDate();
    CPPUNIT_ASSERT(aResult.first);
    CPPUNIT_ASSERT_EQUAL(48000.0, aResult.second);
    CPPUNIT_ASSERT_EQUAL(OUString("2031/06/01"), pFieldmark->GetContent());
    // Current date param contains date in a "standard format"
    OUString sCurrentDate;
    auto pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    CPPUNIT_ASSERT_EQUAL(OUString("2031-06-01"), sCurrentDate);
}

#if !defined(_WIN32)
CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testDateFormFieldCurrentDateInvalidation)
{
    SwDoc* pDoc = createSwDoc();
    CPPUNIT_ASSERT(pDoc);
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT(pMarkAccess);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pMarkAccess->getAllMarksCount());

    // Insert a date form field
    dispatchCommand(mxComponent, ".uno:DatePickerFormField", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());

    // Check whether the fieldmark is created
    auto aIter = pMarkAccess->getAllMarksBegin();
    CPPUNIT_ASSERT(aIter != pMarkAccess->getAllMarksEnd());
    ::sw::mark::IDateFieldmark* pFieldmark = dynamic_cast<::sw::mark::IDateFieldmark*>(*aIter);
    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

    // Set a date first
    sw::mark::IFieldmark::parameter_map_t* pParameters = pFieldmark->GetParameters();
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
    CPPUNIT_ASSERT_EQUAL(OUString("2031-06-01"), sCurrentDate);

    // Now change the content of the field
    pFieldmark->ReplaceContent("[select date]");
    // Do the layouting to trigger invalidation
    calcLayout();
    Scheduler::ProcessEventsToIdle();

    sCurrentDate.clear();
    pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
    if (pResult != pParameters->end())
    {
        pResult->second >>= sCurrentDate;
    }
    CPPUNIT_ASSERT_EQUAL(OUString(""), sCurrentDate);
}
#endif

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testOleSaveWhileEdit)
{
    // Enable LOK mode, otherwise OCommonEmbeddedObject::SwitchStateTo_Impl() will throw when it
    // finds out that the test runs headless.
    comphelper::LibreOfficeKit::setActive();

    // Load a document with a Draw doc in it.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "ole-save-while-edit.odt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->GotoObj(/*bNext=*/true, GotoObjFlags::Any);

    // Select the frame and switch to the frame shell.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->StopShellTimer();

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
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Object 1"));

    // Dispose the document while LOK is still active to avoid leaks.
    mxComponent->dispose();
    mxComponent.clear();
    comphelper::LibreOfficeKit::setActive(false);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf105330)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf105330.odt");

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    SwView* pView = pDoc->GetDocShell()->GetView();
    SfxUInt16Item aRows(SID_ATTR_TABLE_ROW, 1);
    SfxUInt16Item aColumns(SID_ATTR_TABLE_COLUMN, 1);
    pView->GetViewFrame()->GetDispatcher()->ExecuteList(FN_INSERT_TABLE, SfxCallMode::SYNCHRON,
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
    createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // Jump to the first cell, selecting its content
    uno::Sequence<beans::PropertyValue> aSearch(comphelper::InitPropertySequence({
        { "SearchItem.SearchString", uno::Any(OUString("a")) },
        { "SearchItem.Backward", uno::Any(false) },
    }));
    dispatchCommand(mxComponent, ".uno:ExecuteSearch", aSearch);

    //  .uno:Cut doesn't remove the table, only the selected content of the first cell
    dispatchCommand(mxComponent, ".uno:Cut", {});

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // .uno:SelectAll selects the whole table, and UNO command Cut cuts it
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Cut", {});

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowDeletion)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

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
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // This was deleted without change tracking
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);

    // Undo, and repeat the previous test, but only with deletion of the text content of the cells
    // (HasTextChangesOnly property will be removed by Undo)

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // table exists again
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // delete table row with enabled change tracking
    dispatchCommand(mxComponent, ".uno:SelectRow", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});

    // Table row still exists
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept the deletion of the content of the first cell
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) still exists
    // (HasTextChangesOnly property wasn't set for table row deletion)
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Undo, and delete the row without change tracking

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // table exists again
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // disable change tracking
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);

    CPPUNIT_ASSERT_MESSAGE("redlining should be off",
                           !pDoc->getIDocumentRedlineAccess().IsRedlineOn());

    // delete table row without change tracking
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // the table (row) was deleted
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowDeletionWithExport)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

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
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Deleted text content with change tracking,
    // but not table deletion
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it and load it back.
    reload("writer8", "tdf60382_tracked_table_deletion.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pDoc = pTextDoc->GetDocShell()->GetWrtShell()->GetDoc();

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    // (working export/import of HasTextChangesOnly)
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowDeletionWithDOCXExport)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

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
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Deleted text content with change tracking,
    // but not table deletion
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it to a DOCX and load it back.
    // Exporting change tracking of the row wasn't supported.
    // Also Manage Changes for the import.
    reload("Office Open XML Text", "tdf79069_tracked_table_deletion.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pDoc = pTextDoc->GetDocShell()->GetWrtShell()->GetDoc();

    // accept the deletion of the content of the first cell
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->AcceptRedline(0);

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept last redline
    pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    // (working export/import of HasTextChangesOnly)
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineDOCXTableInsertion)
{
    // load a 3-row table inserted with change tracking by text to table conversion
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "TC-table-converttotable.docx");

    // check table count (1)
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // reject the text insertions of the table cells (also reject deletion of the tabulated
    // text source of the table, which was used by the tracked text to table conversion)
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(10), pEditShell->GetRedlineCount());
    while (pEditShell->GetRedlineCount())
        pEditShell->RejectRedline(0);

    // rejecting all text insertions must undo the table insertion
    // This was 1 (remaining empty table after rejecting all table text insertions)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineDOCXTableMoveToFrame)
{
    // load a table with tracked drag & drop: Table1 is the moveFrom,
    // Table2 is the moveTo - and framed - table
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "TC-table-DnD-move.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xTableNames = xTextTablesSupplier->getTextTables();
    // check table count (2)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTables->getCount());

    // accept tracked table moving, remaining table is Table2
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT(xTableNames->hasByName("Table2"));
    CPPUNIT_ASSERT(!xTableNames->hasByName("Table1"));

    // Undo and reject tracked table moving, remaining table is Table1
    dispatchCommand(mxComponent, ".uno:Undo", {});
    rIDRA.AcceptAllRedline(false);
    // This was 2 (not deleted Table2 – framed)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    CPPUNIT_ASSERT(xTableNames->hasByName("Table1"));
    CPPUNIT_ASSERT(!xTableNames->hasByName("Table2"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143215)
{
    // load a table with tracked insertion of an empty row
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "TC-table-rowadd.docx");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // check table count
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // reject insertion of the empty table row
    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(false);
    // This was 4 (remained empty row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());

    // Undo and accept insertion of the table row
    dispatchCommand(mxComponent, ".uno:Undo", {});
    rIDRA.AcceptAllRedline(true);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // delete it with change tracking, and accept the deletion
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());
    rIDRA.AcceptAllRedline(true);
    // This was 4 (remained empty row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf144748)
{
    // load a table with an empty row, and an empty line before the table
    // (to allow the easy selection of the full text with the table)
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf144748.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // there is a table in the text
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // delete full text with the table and check Undo

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    // this crashed LibreOffice
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // redo and check redline usage

    dispatchCommand(mxComponent, ".uno:Redo", {});
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // This was 2 (bad extra redline for the empty row of the deleted table)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(1), pEditShell->GetRedlineCount());

    // accept deletion of the text, including the table with the empty row

    IDocumentRedlineAccess& rIDRA(pDoc->getIDocumentRedlineAccess());
    rIDRA.AcceptAllRedline(true);

    // no table left in the text
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xTables->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf128335)
{
    // Load the bugdoc, which has 3 textboxes.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf128335.odt");

    // Select the 3rd textbox.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Cut it.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);

    // Paste it: this makes the 3rd textbox anchored in the 2nd one.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_PASTE, SfxCallMode::SYNCHRON);

    // Select all shapes.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(xModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    xSelectionSupplier->select(pXTextDocument->getDrawPages()->getByIndex(0));

    // Cut them.
    // Without the accompanying fix in place, this test would have crashed as the textboxes were
    // deleted in an incorrect order.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowDeletionWithReject)
{
    // load a 1-row table, and delete the row with enabled change tracking:
    // now the row is not deleted silently, but keeps the deleted cell contents,
    // and only accepting all of them will result the deletion of the table row.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

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
    dispatchCommand(mxComponent, ".uno:DeleteRows", {});

    // Deleted text content with change tracking,
    // but not table deletion
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // Save it and load it back.
    reload("writer8", "tdf60382_tracked_table_deletion.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pDoc = pTextDoc->GetDocShell()->GetWrtShell()->GetDoc();

    // reject the deletion of the content of the first cell
    // HasTextChangesOnly property of the table row will be true
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(0);

    // Select and delete the content of the first cell
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});

    // table row was still not deleted
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // This was table row deletion instead of remaining the empty row
    // (HasTextChangesOnly was false)
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // restore HasTextChangesOnly = false
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});

    // accept all redlines
    while (pEditShell->GetRedlineCount())
        pEditShell->AcceptRedline(0);

    // table row (and the 1-row table) was deleted finally
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab", 0);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testRedlineTableRowInsertionWithReject)
{
    // load a 1-row table, and insert a row with enabled change tracking
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table and its single row
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);

    // insert rows before and after with enabled change tracking
    // (HasTextChangesOnly property of the row will be false, and
    // add dummy characters CH_TXT_TRACKED_DUMMY_CHAR)
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});
    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});

    SwEditShell* const pEditShell(pDoc->GetEditShell());
    // This was 0 (not tracked row insertion)
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);

    // reject redlines
    pEditShell->RejectRedline(0);
    pEditShell->RejectRedline(0);

    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    // This was 3 (not rejected row insertion)
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf145089_RedlineTableRowInsertionDOCX)
{
    // load a 1-row table, and insert a row with enabled change tracking
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table and its single row
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);

    // insert rows before and after with enabled change tracking
    // (HasTextChangesOnly property of the row will be false, and
    // add dummy characters CH_TXT_TRACKED_DUMMY_CHAR)
    dispatchCommand(mxComponent, ".uno:InsertRowsBefore", {});
    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});

    // save it to DOCX
    reload("Office Open XML Text", "tdf145089.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();
    discardDumpedLayout();
    pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);

    // reject redlines
    SwDoc* pDOCXDoc(pTextDoc->GetDocShell()->GetDoc());
    SwEditShell* const pEditShell(pDOCXDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), pEditShell->GetRedlineCount());
    pEditShell->RejectRedline(0);
    pEditShell->RejectRedline(0);

    discardDumpedLayout();

    pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//page[1]//body/tab");

    // This was 3 (not rejected row insertion)
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 1);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testPasteTrackedTableRow)
{
    // load a 1-row table
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table count
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());

    // copy table row and paste it by Paste Special->Rows Above
    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:Escape", {});
    dispatchCommand(mxComponent, ".uno:PasteRowsBefore", {});

    // 2-row table
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());

    // This was 2 (inserted as a nested table in the first cell of the new row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // Is it a tracked row insertion? Its rejection results the original 1-row table
    dispatchCommand(mxComponent, ".uno:RejectAllTrackedChanges", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testPasteTrackedTableRowInHideChangesMode)
{
    // load a 1-row table
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf118311.fodt");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE("redlines should be invisible",
                           !IDocumentRedlineAccess::IsShowChanges(
                               pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check table count
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    // check table row count
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());

    // copy table row and paste it by Paste Special->Rows Above
    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:Escape", {});

    // This resulted freezing
    dispatchCommand(mxComponent, ".uno:PasteRowsBefore", {});

    // 2-row table
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());

    // This was 2 (inserted as a nested table in the first cell of the new row)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {}); // FIXME Why 3 Undos?
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTable->getRows()->getCount());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf146966)
{
    // load a 4-row table, select more than 1 row and copy them
    // to check insertion of unnecessary empty rows
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf144748.fodt");

    // check table row count
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    // copy table row and paste it by Paste Special->Rows Above
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    dispatchCommand(mxComponent, ".uno:SelectTable", {});
    dispatchCommand(mxComponent, ".uno:Copy", {});
    dispatchCommand(mxComponent, ".uno:Escape", {});
    dispatchCommand(mxComponent, ".uno:PasteRowsBefore", {});

    // This was 35 (extra empty rows)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:Undo", {}); // FIXME Why 3 Undos?
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xTable->getRows()->getCount());

    dispatchCommand(mxComponent, ".uno:Redo", {});
    dispatchCommand(mxComponent, ".uno:Redo", {});
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xTable->getRows()->getCount());
    // dispatchCommand(mxComponent, ".uno:Redo", {}); // FIXME assert
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf145091)
{
    // load a deleted table, reject them, and delete only its text and export to DOCX
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf145091.docx");

    // turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // reject all redlines
    SwEditShell* const pEditShell(pDoc->GetEditShell());
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(3), pEditShell->GetRedlineCount());
    while (pEditShell->GetRedlineCount() > 0)
        pEditShell->RejectRedline(0);
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(0), pEditShell->GetRedlineCount());

    // delete only table text, but not table
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:Delete", {});
    CPPUNIT_ASSERT(pEditShell->GetRedlineCount() > 0);

    // save it to DOCX
    reload("Office Open XML Text", "tdf145091.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwViewShell* pViewShell
        = pTextDoc->GetDocShell()->GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell();
    pViewShell->Reformat();
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);

    // accept all redlines
    dispatchCommand(mxComponent, ".uno:AcceptAllTrackedChanges", {});

    discardDumpedLayout();

    pXmlDoc = parseLayoutDump();
    // This was false (deleted table with accepting deletions)
    assertXPath(pXmlDoc, "//page[1]//body/tab");
    assertXPath(pXmlDoc, "//page[1]//body/tab/row", 3);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf128603)
{
    // Load the bugdoc, which has 3 textboxes.
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf128603.odt");

    // Select the 3rd textbox.
    SwView* pView = pDoc->GetDocShell()->GetView();
    pView->GetViewFrame()->GetDispatcher()->Execute(FN_CNTNT_TO_NEXT_FRAME, SfxCallMode::SYNCHRON);
    // Make sure SwTextShell is replaced with SwDrawShell right now, not after 120 ms, as set in the
    // SwView ctor.
    pView->StopShellTimer();
    SwXTextDocument* pXTextDocument = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_TAB);
    pXTextDocument->postKeyEvent(LOK_KEYEVENT_KEYUP, 0, KEY_TAB);
    Scheduler::ProcessEventsToIdle();

    // Cut it.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_CUT, SfxCallMode::SYNCHRON);

    // Paste it: this makes the 3rd textbox anchored in the 2nd one.
    pView->GetViewFrame()->GetDispatcher()->Execute(SID_PASTE, SfxCallMode::SYNCHRON);

    // Undo all of this.
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();
    rUndoManager.Undo();

    // Make sure the content indexes still match.
    const SwFrameFormats& rSpzFrameFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(6), rSpzFrameFormats.size());
    const SwNodeIndex* pIndex4 = rSpzFrameFormats[4]->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pIndex4);
    const SwNodeIndex* pIndex5 = rSpzFrameFormats[5]->GetContent().GetContentIdx();
    CPPUNIT_ASSERT(pIndex5);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 11
    // - Actual  : 14
    // i.e. the shape content index and the frame content index did not match after undo, even if
    // their "other text box format" pointers pointed to each other.
    CPPUNIT_ASSERT_EQUAL(pIndex4->GetIndex(), pIndex5->GetIndex());
}

// only care that it doesn't assert/crash
CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testOfz18563)
{
    OUString sURL(m_directories.getURLFromSrc(u"/sw/qa/extras/uiwriter/data/ofz18563.docx"));
    SvFileStream aFileStream(sURL, StreamMode::READ);
    TestImportDOCX(aFileStream);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf143904)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf143904.odt");

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});
    pWrtShell->Down(false);
    pWrtShell->Insert("foo");

    SwTextNode* pTextNodeA1 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT(pTextNodeA1->GetText().startsWith("Insert"));
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNodeA2 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pTextNodeA2->GetText());
    CPPUNIT_ASSERT_EQUAL(false, pTextNodeA2->GetSwAttrSet().HasItem(RES_CHRATR_FONT));
    OUString sFontName = pTextNodeA2->GetSwAttrSet().GetItem(RES_CHRATR_FONT)->GetFamilyName();
    CPPUNIT_ASSERT_EQUAL(OUString("Liberation Serif"), sFontName);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf90069)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf90069.docx");

    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();

    dispatchCommand(mxComponent, ".uno:InsertRowsAfter", {});
    pWrtShell->Down(false);
    pWrtShell->Insert("foo");

    SwTextNode* pTextNodeA1 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT(pTextNodeA1->GetText().startsWith("Insert"));
    nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNodeA2 = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);
    CPPUNIT_ASSERT_EQUAL(OUString("foo"), pTextNodeA2->GetText());
    CPPUNIT_ASSERT_EQUAL(true, pTextNodeA2->GetSwAttrSet().HasItem(RES_CHRATR_FONT));
    OUString sFontName = pTextNodeA2->GetSwAttrSet().GetItem(RES_CHRATR_FONT)->GetFamilyName();
    CPPUNIT_ASSERT_EQUAL(OUString("Lohit Devanagari"), sFontName);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf109266)
{
    // transliteration with redlining
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "lorem.fodt");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT(pWrtShell);

    SwNodeOffset nIndex = pWrtShell->GetCursor()->GetNode().GetIndex();
    SwTextNode* pTextNode = static_cast<SwTextNode*>(pDoc->GetNodes()[nIndex]);

    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum..."), pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToTitleCase", {});

    CPPUNIT_ASSERT_EQUAL(OUString("Lorem Ipsum..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem Ipsum..."), pTextNode->GetRedlineText());

    //turn on red-lining and show changes
    RedlineFlags const mode(pWrtShell->GetRedlineFlags() | RedlineFlags::On);
    CPPUNIT_ASSERT(mode & (RedlineFlags::ShowDelete | RedlineFlags::ShowInsert));
    pWrtShell->SetRedlineFlags(mode);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToTitleCase", {});

    // This was "Lorem Ipsum..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsumIpsum..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem Ipsum..."), pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToUpper", {});

    // This was "LOREM IPSUM..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum...LOREM IPSUM..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("LOREM IPSUM..."), pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToLower", {});

    // This was "lorem ipsum..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum...lorem ipsum..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("lorem ipsum..."), pTextNode->GetRedlineText());

    dispatchCommand(mxComponent, ".uno:Undo", {});
    dispatchCommand(mxComponent, ".uno:SelectAll", {});
    dispatchCommand(mxComponent, ".uno:ChangeCaseToToggleCase", {});

    // This was "lOREM IPSUM..." (missing redlining)
    CPPUNIT_ASSERT_EQUAL(OUString("Lorem ipsum...lOREM IPSUM..."), pTextNode->GetText());
    CPPUNIT_ASSERT_EQUAL(OUString("lOREM IPSUM..."), pTextNode->GetRedlineText());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf129655)
{
    createSwDoc(DATA_DIRECTORY, "tdf129655-vtextbox.odt");
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "//anchored/fly/txt[@WritingMode='Vertical']", 1);
}

static uno::Reference<text::XTextRange> getAssociatedTextRange(uno::Any object)
{
    // possible cases:
    // 1. a container of other objects - e.g. selection of 0 to n text portions, or 1 to n drawing objects
    try
    {
        uno::Reference<container::XIndexAccess> xIndexAccess(object, uno::UNO_QUERY_THROW);
        if (xIndexAccess.is() && xIndexAccess->getCount() > 0)
        {
            for (int i = 0; i < xIndexAccess->getCount(); ++i)
            {
                uno::Reference<text::XTextRange> xRange
                    = getAssociatedTextRange(xIndexAccess->getByIndex(i));
                if (xRange.is())
                    return xRange;
            }
        }
    }
    catch (const uno::Exception&)
    {
    }

    // 2. another TextContent, having an anchor we can use
    try
    {
        uno::Reference<text::XTextContent> xTextContent(object, uno::UNO_QUERY_THROW);
        if (xTextContent.is())
        {
            uno::Reference<text::XTextRange> xRange = xTextContent->getAnchor();
            if (xRange.is())
                return xRange;
        }
    }
    catch (const uno::Exception&)
    {
    }

    // an object which supports XTextRange directly
    try
    {
        uno::Reference<text::XTextRange> xRange(object, uno::UNO_QUERY_THROW);
        if (xRange.is())
            return xRange;
    }
    catch (const uno::Exception&)
    {
    }

    return nullptr;
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf123218)
{
    struct ReverseXAxisOrientationDoughnutChart
        : public comphelper::ConfigurationProperty<ReverseXAxisOrientationDoughnutChart, bool>
    {
        static OUString path()
        {
            return "/org.openoffice.Office.Compatibility/View/ReverseXAxisOrientationDoughnutChart";
        }
        ~ReverseXAxisOrientationDoughnutChart() = delete;
    };

    struct ClockwisePieChartDirection
        : public comphelper::ConfigurationProperty<ClockwisePieChartDirection, bool>
    {
        static OUString path()
        {
            return "/org.openoffice.Office.Compatibility/View/ClockwisePieChartDirection";
        }
        ~ClockwisePieChartDirection() = delete;
    };
    auto batch = comphelper::ConfigurationChanges::create();

    ReverseXAxisOrientationDoughnutChart::set(false, batch);
    ClockwisePieChartDirection::set(true, batch);
    batch->commit();

    createSwDoc();

    // create an OLE shape in the document
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xMSF);
    uno::Reference<beans::XPropertySet> xShapeProps(
        xMSF->createInstance("com.sun.star.text.TextEmbeddedObject"), uno::UNO_QUERY);
    xShapeProps->setPropertyValue("CLSID",
                                  uno::Any(OUString("12dcae26-281f-416f-a234-c3086127382e")));
    uno::Reference<drawing::XShape> xShape(xShapeProps, uno::UNO_QUERY_THROW);
    xShape->setSize(awt::Size(16000, 9000));
    uno::Reference<text::XTextContent> chartTextContent(xShapeProps, uno::UNO_QUERY_THROW);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    uno::Any aSelection = xSelSupplier->getSelection();
    uno::Reference<text::XTextRange> xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    xTextRange->getText()->insertTextContent(xTextRange, chartTextContent, false);

    // insert a doughnut chart
    uno::Reference<frame::XModel> xDocModel;
    xShapeProps->getPropertyValue("Model") >>= xDocModel;
    CPPUNIT_ASSERT(xDocModel);
    uno::Reference<chart::XChartDocument> xChartDoc(xDocModel, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc);
    uno::Reference<lang::XMultiServiceFactory> xChartMSF(xChartDoc, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartMSF);
    uno::Reference<chart::XDiagram> xDiagram(
        xChartMSF->createInstance("com.sun.star.chart.DonutDiagram"), uno::UNO_QUERY);
    xChartDoc->setDiagram(xDiagram);

    // test primary X axis Orientation value
    uno::Reference<chart2::XChartDocument> xChartDoc2(xChartDoc, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xChartDoc2);
    uno::Reference<chart2::XCoordinateSystemContainer> xCooSysContainer(
        xChartDoc2->getFirstDiagram(), uno::UNO_QUERY_THROW);
    uno::Sequence<uno::Reference<chart2::XCoordinateSystem>> xCooSysSequence
        = xCooSysContainer->getCoordinateSystems();
    uno::Reference<chart2::XCoordinateSystem> xCoord = xCooSysSequence[0];
    CPPUNIT_ASSERT(xCoord.is());
    uno::Reference<chart2::XAxis> xAxis = xCoord->getAxisByDimension(0, 0);
    CPPUNIT_ASSERT(xAxis.is());
    chart2::ScaleData aScaleData = xAxis->getScaleData();
    CPPUNIT_ASSERT_EQUAL(chart2::AxisOrientation_MATHEMATICAL, aScaleData.Orientation);

    // tdf#108059 test primary Y axis Orientation value
    uno::Reference<chart2::XAxis> xYAxis = xCoord->getAxisByDimension(1, 0);
    CPPUNIT_ASSERT(xYAxis.is());
    aScaleData = xYAxis->getScaleData();
    CPPUNIT_ASSERT_EQUAL(chart2::AxisOrientation_REVERSE, aScaleData.Orientation);
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf93747)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtSh = pDoc->GetDocShell()->GetWrtShell();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    pWrtSh->Insert("Col1");

    // Move the cursor to B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    pWrtSh->Insert("Col2");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Col1"), xCellA1->getString());

    uno::Reference<text::XTextRange> xCellB1(xTextTable->getCellByName("B1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Col2"), xCellB1->getString());

    // Select backwards B1 and A1
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);

    // Just select the whole B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    uno::Sequence<beans::PropertyValue> aPropertyValues = comphelper::InitPropertySequence({
        { "Style", uno::Any(OUString("Heading 1")) },
        { "FamilyName", uno::Any(OUString("ParagraphStyles")) },
    });
    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    // Without the fix in place, this test would have failed with
    // - Expected: Table Contents
    // - Actual  : Heading 1
    CPPUNIT_ASSERT_EQUAL(
        OUString("Table Contents"),
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), "ParaStyleName"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("Heading 1"),
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "ParaStyleName"));

    // Now select A1 again
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, ".uno:StyleApply", aPropertyValues);

    CPPUNIT_ASSERT_EQUAL(
        OUString("Heading 1"),
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), "ParaStyleName"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("Heading 1"),
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "ParaStyleName"));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(
        OUString("Table Contents"),
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), "ParaStyleName"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("Heading 1"),
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "ParaStyleName"));

    dispatchCommand(mxComponent, ".uno:Undo", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(
        OUString("Table Contents"),
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), "ParaStyleName"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("Table Contents"),
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "ParaStyleName"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf145151)
{
    SwDoc* pDoc = createSwDoc();
    SwWrtShell* pWrtSh = pDoc->GetDocShell()->GetWrtShell();

    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
        { { "Rows", uno::Any(sal_Int32(2)) }, { "Columns", uno::Any(sal_Int32(2)) } }));

    dispatchCommand(mxComponent, ".uno:InsertTable", aArgs);
    Scheduler::ProcessEventsToIdle();

    pWrtSh->Insert("Col1");

    // Move the cursor to B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);

    pWrtSh->Insert("Col2");

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getRows()->getCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xTextTable->getColumns()->getCount());

    uno::Reference<text::XTextRange> xCellA1(xTextTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Col1"), xCellA1->getString());

    uno::Reference<text::XTextRange> xCellB1(xTextTable->getCellByName("B1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Col2"), xCellB1->getString());

    // Select backwards B1 and A1 (select "2loC<cell>" which ends up selecting both cells)
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 5, /*bBasicCall=*/false);

    // Just select the whole B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    dispatchCommand(mxComponent, ".uno:DefaultNumbering", {});
    Scheduler::ProcessEventsToIdle();

    // B1 should now have a numbering style, but A1 should not be affected.
    OUString sNumStyleB1
        = getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "NumberingStyleName");
    CPPUNIT_ASSERT(!sNumStyleB1.isEmpty());
    CPPUNIT_ASSERT_MESSAGE(
        "Only cell B1 was selected. A1 should not have any numbering.",
        getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), "NumberingStyleName")
            .isEmpty());

    // Toggle it back off
    dispatchCommand(mxComponent, ".uno:DefaultNumbering", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_MESSAGE(
        "Cell B1 must be able to toggle numbering on and off.",
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "NumberingStyleName")
            .isEmpty());

    // Now test removing numbering/bullets
    // Add A1 to the current B1 selection
    pWrtSh->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // Toggle on bullet numbering
    dispatchCommand(mxComponent, ".uno:DefaultBullet", {});
    Scheduler::ProcessEventsToIdle();

    // sanity check - both cells have bullets turned on.
    OUString sNumStyleA1
        = getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), "NumberingStyleName");
    CPPUNIT_ASSERT(!sNumStyleA1.isEmpty());
    CPPUNIT_ASSERT_EQUAL(
        sNumStyleA1,
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "NumberingStyleName"));
    CPPUNIT_ASSERT(sNumStyleA1 != sNumStyleB1); // therefore B1 changed from numbering to bullets

    // Just select cell B1
    pWrtSh->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);

    // Toggle off bullet numbering
    dispatchCommand(mxComponent, ".uno:DefaultBullet", {});
    Scheduler::ProcessEventsToIdle();

    // B1 should now have removed all numbering, while A1 should still have the bullet.
    CPPUNIT_ASSERT(
        getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "NumberingStyleName")
            .isEmpty());
    CPPUNIT_ASSERT_MESSAGE(
        "Only cell B1 was selected. A1 should still have bullets turned on.",
        !getProperty<OUString>(getParagraphOfText(1, xCellA1->getText()), "NumberingStyleName")
             .isEmpty());

    // Toggle it back on
    dispatchCommand(mxComponent, ".uno:DefaultBullet", {});
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT(
        !getProperty<OUString>(getParagraphOfText(1, xCellB1->getText()), "NumberingStyleName")
             .isEmpty());
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest5, testTdf126735)
{
    SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "tdf39721.fodt");

    //turn on red-lining and show changes
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowDelete
                                                      | RedlineFlags::ShowInsert);
    CPPUNIT_ASSERT_MESSAGE("redlining should be on",
                           pDoc->getIDocumentRedlineAccess().IsRedlineOn());
    CPPUNIT_ASSERT_MESSAGE(
        "redlines should be visible",
        IDocumentRedlineAccess::IsShowChanges(pDoc->getIDocumentRedlineAccess().GetRedlineFlags()));

    // check next selected tracked change
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", {});
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XSelectionSupplier> xSelSupplier(xModel->getCurrentController(),
                                                          uno::UNO_QUERY_THROW);
    uno::Any aSelection = xSelSupplier->getSelection();
    uno::Reference<text::XTextRange> xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    CPPUNIT_ASSERT_EQUAL(OUString(" ipsu"), xTextRange->getString());

    // check next selected tracked change
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    CPPUNIT_ASSERT_EQUAL(OUString("or "), xTextRange->getString());

    // check next selected tracked change at the end of the document:
    // select the first tracked change of the document
    dispatchCommand(mxComponent, ".uno:NextTrackedChange", {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    // This was empty (collapsing at the end of the last tracked change)
    CPPUNIT_ASSERT_EQUAL(OUString(" ipsu"), xTextRange->getString());

    // check the previous tracked change at the start of the document:
    // select the last tracked change of the document
    dispatchCommand(mxComponent, ".uno:PreviousTrackedChange", {});
    aSelection = xSelSupplier->getSelection();
    xTextRange = getAssociatedTextRange(aSelection);
    CPPUNIT_ASSERT(xTextRange);
    // This was empty (collapsing at the start of the last tracked change)
    CPPUNIT_ASSERT_EQUAL(OUString("or "), xTextRange->getString());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
