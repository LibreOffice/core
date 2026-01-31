/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <officecfg/Office/Writer.hxx>
#include <vcl/pdf/PDFPageObjectType.hxx>
#include <vcl/scheduler.hxx>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/configuration.hxx>

#include <AnnotationWin.hxx>
#include <edtwin.hxx>
#include <postithelper.hxx>
#include <PostItMgr.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <unotxdoc.hxx>
#include <ndtxt.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <svx/svxids.hrc>

namespace
{
class SwUiWriterTest11 : public SwModelTestBase
{
public:
    SwUiWriterTest11()
        : SwModelTestBase(u"/sw/qa/extras/uiwriter/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf167760_numberedPara)
{
    createSwDoc("tdf167760_numberedPara.odt");

    CPPUNIT_ASSERT_EQUAL(OUString("1.1."),
                         getProperty<OUString>(getParagraph(1), "ListLabelString"));

    // apply a non-numbered style to paragraph 1
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Text body"_ustr) });
    // the numbering should not be affected by changing a paragraph style
    CPPUNIT_ASSERT_EQUAL(OUString("1.1."),
                         getProperty<OUString>(getParagraph(1), "ListLabelString"));

    // apply a numbered style
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"aList"_ustr) });
    // the numbering should be changed to that specified by the paragraph style
    CPPUNIT_ASSERT_EQUAL(OUString("i.I.a)"),
                         getProperty<OUString>(getParagraph(1), "ListLabelString"));

    // apply the non-numbered style while holding down the Ctrl-key
    dispatchCommand(mxComponent, u".uno:StyleApply"_ustr,
                    { comphelper::makePropertyValue(u"FamilyName"_ustr, u"ParagraphStyles"_ustr),
                      comphelper::makePropertyValue(u"Style"_ustr, u"Text body"_ustr),
                      comphelper::makePropertyValue(u"KeyModifier"_ustr, uno::Any(KEY_MOD1)) });
    // the numbering should be removed when the Ctrl-key is held down
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), "ListLabelString"));
}

CPPUNIT_TEST_FIXTURE(SwUiWriterTest11, testTdf163194)
{
    // Test (1) that exporting comments in margin to PDF produces expected layout of the comments,
    // that have limited and reasonable width and position, inside the page bounds (and close to
    // its right side), independent on the manually set comment sidebar width; and (2) that the
    // export doesn't change the annotation sizes in the document.

    // Annotation size depends on DisplayWidthFactor. Set it to a fixed value for testing.
    comphelper::ScopeGuard
        aReset([oldValue = officecfg::Office::Writer::Notes::DisplayWidthFactor::get()]() {
            auto pChanges(comphelper::ConfigurationChanges::create());
            officecfg::Office::Writer::Notes::DisplayWidthFactor::set(oldValue, pChanges);
            pChanges->commit();
        });
    {
        auto pChanges(comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::Notes::DisplayWidthFactor::set(4.0, pChanges);
        pChanges->commit();
    }

    createSwDoc("tdf163194-two-comments.fodt");

    SwDocShell* pDocShell = getSwDocShell();
    CPPUNIT_ASSERT(pDocShell);
    SwView* pView = pDocShell->GetView();
    CPPUNIT_ASSERT(pView);
    SwPostItMgr* pPostItMgr = pView->GetPostItMgr();
    CPPUNIT_ASSERT(pPostItMgr);

    CPPUNIT_ASSERT_EQUAL(size_t(2), pPostItMgr->GetPostItFields().size());
    std::vector<tools::Long> aOriginalHeights;
    for (const auto& pAnnotationItem : pPostItMgr->GetPostItFields())
    {
        CPPUNIT_ASSERT(pAnnotationItem);
        CPPUNIT_ASSERT(pAnnotationItem->mpPostIt);
        const Size aSize = pAnnotationItem->mpPostIt->GetSizePixel();
        // Pixel width depends on DisplayWidthFactor, seems to not depend on DPI
        CPPUNIT_ASSERT_EQUAL(tools::Long(400), aSize.Width());
        // Heights depend on width and current DPI scaling; just remember them for later comparison
        aOriginalHeights.push_back(aSize.Height());
    }

    // Export to PDF with comments in margin
    uno::Sequence aFilterData{ comphelper::makePropertyValue(u"ExportNotes"_ustr, false),
                               comphelper::makePropertyValue(u"ExportNotesInMargin"_ustr, true) };
    uno::Sequence aDescriptor{ comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
                               comphelper::makePropertyValue(u"URL"_ustr, maTempFile.GetURL()) };
    dispatchCommand(mxComponent, u".uno:ExportToPDF"_ustr, aDescriptor);

    if (auto pPdfDocument = parsePDFExport()) // This part will be skipped without PDFium
    {
        CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());
        auto pPage = pPdfDocument->openPage(0);
        CPPUNIT_ASSERT(pPage);
        // 595 pt corresponts to 210 mm (withh of A4)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(595, pPage->getWidth(), 1.0);
        auto pTextPage = pPage->getTextPage();
        CPPUNIT_ASSERT(pTextPage);

        // There should be two filled path objects corresponding to the two annotations
        int nPathCount = 0;

        // Check that there are all the expected lines of the annotations, i.e. that the layout
        // of the comments is stable and independent of the DPI.
        const OUString aLines[] = {
            // First annotation
            u"A comment. One two three four "_ustr,
            u"five six seven eight nine ten "_ustr,
            u"eleven twelve thirteen fourteen "_ustr,
            u"fifteen sixteen seventeen "_ustr,
            u"eighteen nineteen twenty."_ustr,
            // Second annotation
            u"Another comment. Twenty-one "_ustr,
            u"twenty-two twenty-three "_ustr,
            u"twenty-four twenty-five twenty-"_ustr,
            u"six twenty-seven twenty-eight "_ustr,
            u"twenty-nine thirty."_ustr,
        };
        std::set<OUString> aFoundLines;

        for (int i = 0; i < pPage->getObjectCount(); ++i)
        {
            auto pObject = pPage->getObject(i);

            if (pObject->getType() == vcl::pdf::PDFPageObjectType::Path
                && pObject->getPathSegmentCount() == 5)
            {
                // This is the filled rectangle of an annotation. I hope that path segment count
                // is stable and unique enough to identify it.
                CPPUNIT_ASSERT(pObject->getFillColor() != COL_TRANSPARENT);
                basegfx::B2DRectangle bounds = pObject->getBounds();
                // The object must start at position 448, and have width 101 (so its right
                // edge is ~at position 549, within the page's width of 595).
                // Without the fix, this failed like "Expected: 101; Actual: 225", i.e. its
                // right edge protruded beyond the right page limit (and generally depended
                // on the UI width of the comment sidebar).
                CPPUNIT_ASSERT_DOUBLES_EQUAL(101, bounds.getWidth(), 1.0);
                CPPUNIT_ASSERT_DOUBLES_EQUAL(448, bounds.getMinX(), 1.0);
                ++nPathCount;
            }
            else if (pObject->getType() == vcl::pdf::PDFPageObjectType::Text)
            {
                // This is text; check if it matches expected annotation lines
                OUString aText = pObject->getText(pTextPage);
                if (std::ranges::find(aLines, aText) != std::end(aLines))
                {
                    // Check that it fits into the expected horizontal range of the comment
                    basegfx::B2DRectangle bounds = pObject->getBounds();
                    CPPUNIT_ASSERT_LESS(101.0, bounds.getWidth());
                    CPPUNIT_ASSERT_DOUBLES_EQUAL(450, bounds.getMinX(), 1.0);

                    CPPUNIT_ASSERT(!aFoundLines.contains(aText)); // each line only once
                    aFoundLines.insert(aText);
                }
            }
        }
        CPPUNIT_ASSERT_EQUAL(2, nPathCount); // all annotation rectangles
        CPPUNIT_ASSERT_EQUAL(std::size(aLines), aFoundLines.size()); // all annotation lines
    }

    // Test that export didn't change the annotation sizes
    CPPUNIT_ASSERT_EQUAL(size_t(2), pPostItMgr->GetPostItFields().size());
    for (size_t i = 0; i < 2; ++i)
    {
        const auto& pAnnotationItem = pPostItMgr->GetPostItFields()[i];
        CPPUNIT_ASSERT(pAnnotationItem);
        CPPUNIT_ASSERT(pAnnotationItem->mpPostIt);
        const Size aSize = pAnnotationItem->mpPostIt->GetSizePixel();
        CPPUNIT_ASSERT_EQUAL(tools::Long(400), aSize.Width());
        CPPUNIT_ASSERT_EQUAL(aOriginalHeights[i], aSize.Height());
    }
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
