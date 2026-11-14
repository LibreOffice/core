/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextSectionsSupplier.hpp>

#include <comphelper/scopeguard.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/hostfilter.hxx>

#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <redline.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <node.hxx>
#include <swtable.hxx>

/// Covers sw/source/core/docnode/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/docnode/data/"_ustr)
    {
    }

    OUString getSectionText(const OUString& rName)
    {
        uno::Reference<css::text::XTextSectionsSupplier> xSupplier(mxComponent,
                                                                   uno::UNO_QUERY_THROW);
        uno::Reference<css::text::XTextContent> xSection(
            xSupplier->getTextSections()->getByName(rName), uno::UNO_QUERY_THROW);
        return xSection->getAnchor()->getString();
    }
};

CPPUNIT_TEST_FIXTURE(Test, testLinkUpdateKeepsSectionOfMissingFile)
{
    // Given a master document whose two linked sections have their content cached, the file of
    // the second one is missing:
    createSwGlobalDoc("master-cached-sections.odm");
    SwDoc* pDoc = getSwDoc();

    // When updating the links:
    pDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, u""_ustr);

    // Then the first section shows its file, and the second keeps its content. Without the fix,
    // the second section was emptied before its file was found missing.
    CPPUNIT_ASSERT_EQUAL(u"Text of the chapter file"_ustr, getSectionText(u"Present"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Cached text of the missing chapter"_ustr,
                         getSectionText(u"Missing"_ustr));
}

// HostFilter::setAllowedExtRefPaths takes colon-separated paths, which do not fit Windows paths.
#ifndef _WIN32
CPPUNIT_TEST_FIXTURE(Test, testLinkUpdateKeepsSectionOfForbiddenFile)
{
    // Given a master document with cached section content, where no file URL may be followed:
    comphelper::ScopeGuard g([]() { HostFilter::resetAllowedExtRefPaths(); });
    HostFilter::setAllowedExtRefPaths("");
    createSwGlobalDoc("master-cached-sections.odm");
    SwDoc* pDoc = getSwDoc();
    pDoc->getIDocumentState().ResetModified();

    // When updating the links:
    pDoc->getIDocumentLinksAdministration().GetLinkManager().UpdateAllLinks(false, u""_ustr);

    // Then both sections keep their content and the document is not modified. Without the fix,
    // both sections were emptied before the file URLs were found forbidden.
    CPPUNIT_ASSERT_EQUAL(u"Cached text of the present chapter"_ustr,
                         getSectionText(u"Present"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Cached text of the missing chapter"_ustr,
                         getSectionText(u"Missing"_ustr));
    CPPUNIT_ASSERT(!pDoc->getIDocumentState().IsModified());
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testRedlineEndsBeforeToC)
{
    // Load a document where a delete redline ends right before a ToC, then redlines are hidden at a
    // layout level.
    createSwDoc("redline-ends-before-toc.docx");
    SwDoc* pDoc = getSwDoc();
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    // Without the accompanying fix in place, this test would have resulted in an assertion failure
    // in InsertCnt_(), because the start of the section was hidden, but not its end.
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(2), rTable.size());

    // The redline contained the newline, too
    CPPUNIT_ASSERT_EQUAL(u"<add-table-of-content>"_ustr, rTable[0]->GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150086)
{
    // Load a document where an insert redline ends right before a ToC
    createSwDoc("tdf150086.docx");
    SwDoc* pDoc = getSwDoc();
    const SwRedlineTable& rTable = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
    CPPUNIT_ASSERT_EQUAL(static_cast<SwRedlineTable::size_type>(8), rTable.size());

    // This was "Conte" (stripped redline)
    CPPUNIT_ASSERT_EQUAL(u"Content\n"_ustr, rTable[6]->GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf156267)
{
    createSwDoc("tdf156267.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    dispatchCommand(mxComponent, u".uno:SelectAll"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Copy"_ustr, {});
    dispatchCommand(mxComponent, u".uno:Paste"_ustr, {});

    // Without the fix in place, it would have crashed here
    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

namespace
{
/// Counts the boxes of all tables in the document, and the cell sections in the document.
std::pair<size_t, int> CountTableBoxesAndCellSections(SwDoc& rDoc)
{
    SwNodes& rNodes = rDoc.GetNodes();
    size_t nBoxes = 0;
    int nCellSections = 0;
    for (SwNodeOffset nNode(0); nNode < rNodes.Count(); ++nNode)
    {
        if (const SwTableNode* pTableNode = rNodes[nNode]->GetTableNode())
            nBoxes += pTableNode->GetTable().GetTabSortBoxes().size();
        const SwStartNode* pStartNode = rNodes[nNode]->GetStartNode();
        if (pStartNode && pStartNode->GetStartNodeType() == SwTableBoxStartNode)
            ++nCellSections;
    }
    return { nBoxes, nCellSections };
}
}

CPPUNIT_TEST_FIXTURE(Test, testTableBoxKeepsItsCellSection)
{
    // Given a document whose tracked deletion carries on across a cell mark, so applying it at
    // import empties one cell of the table:
    createSwDoc("ofz515655013.doc");

    // Then every box of every table still has a cell section of its own:
    auto[nBoxes, nCellSections] = CountTableBoxesAndCellSections(*getSwDoc());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(26), nBoxes);
    CPPUNIT_ASSERT_EQUAL(26, nCellSections);
}

CPPUNIT_TEST_FIXTURE(Test, testTableKeepsCellInsideDeletion)
{
    // Given a document whose tracked deletion starts in one cell and ends two cells later, so the
    // cell between them lies wholly inside the deletion:
    createSwDoc("ofz566727923.doc");

    // Then every box of every table still has a cell section of its own:
    auto[nBoxes, nCellSections] = CountTableBoxesAndCellSections(*getSwDoc());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(25), nBoxes);
    CPPUNIT_ASSERT_EQUAL(25, nCellSections);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
