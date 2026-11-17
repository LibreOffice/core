/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextSectionsSupplier.hpp>

#include <comphelper/kit.hxx>
#include <comphelper/scopeguard.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <tools/urlobj.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/hostfilter.hxx>
#include <tools/json_writer.hxx>
#include <vcl/scheduler.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <unotxdoc.hxx>
#include <docsh.hxx>

#include <IDocumentLinksAdministration.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentState.hxx>
#include <redline.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <node.hxx>
#include <swtable.hxx>
#include <section.hxx>

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
        cpo::uno::Reference<css::text::XTextSectionsSupplier> xSupplier(
            mxComponent, cpo::uno::UNO_QUERY_THROW);
        cpo::uno::Reference<css::text::XTextContent> xSection(
            xSupplier->getTextSections()->getByName(rName), cpo::uno::UNO_QUERY_THROW);
        return xSection->getAnchor()->getString();
    }

    // Whether the section of that name shows its file, or is a broken link.
    bool isSectionConnected(std::u16string_view rName)
    {
        const SwSection* pFound = nullptr;
        for (const SwSectionFormat* pFormat : getSwDoc()->GetSections())
        {
            const SwSection* pSection = pFormat->GetSection();
            if (pSection && pSection->GetSectionName() == rName)
            {
                pFound = pSection;
                break;
            }
        }
        CPPUNIT_ASSERT_MESSAGE("no such section", pFound);
        return pFound->IsConnectFlag();
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
    // And the second section is a broken link. Without the fix in place, keeping the content
    // also kept it connected, so the Navigator did not show the file as missing.
    CPPUNIT_ASSERT(isSectionConnected(u"Present"));
    CPPUNIT_ASSERT(!isSectionConnected(u"Missing"));
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
    // And both are broken links.
    CPPUNIT_ASSERT(!isSectionConnected(u"Present"));
    CPPUNIT_ASSERT(!isSectionConnected(u"Missing"));
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testLinkedFilesOfWorkingCopy)
{
    // Given a master document loaded from a working copy somewhere else, as an app does that loads
    // a temporary copy of the file the user opened:
    createTempCopy(u"master-cached-sections.odm");
    const OUString aWorkingURL
        = INetURLObject(maTempFile.GetURL()).GetMainURL(INetURLObject::DecodeMechanism::NONE);
    comphelper::COKit::setOriginalDocumentUrl(aWorkingURL,
                                              createFileURL(u"master-cached-sections.odm"));
    comphelper::ScopeGuard g(
        [&aWorkingURL]() { comphelper::COKit::clearOriginalDocumentUrl(aWorkingURL); });
    loadFromURL(maTempFile.GetURL());

    // Then the links are relative to the file the user opened, so the chapter next to it is found.
    // Without the fix in place, they were relative to the working copy, where there is no chapter.
    tools::JsonWriter aJson;
    CPPUNIT_ASSERT(getSwTextDoc()->getLinkedFiles(aJson));
    std::stringstream aStream{ std::string(aJson.finishAndGetAsOString()) };
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    const boost::property_tree::ptree& rFirst = aTree.get_child("links").front().second;
    CPPUNIT_ASSERT_EQUAL(std::string("master-chapter.odt"), rFirst.get<std::string>("id"));
    CPPUNIT_ASSERT(rFirst.get<bool>("readable"));

    getSwDocShell()->AllowLinkUpdate();
    Scheduler::ProcessEventsToIdle();
    CPPUNIT_ASSERT_EQUAL(u"Text of the chapter file"_ustr, getSectionText(u"Present"_ustr));

    // And when the working copy is saved, the link is still relative to the file the user opened.
    // Without the fix in place, it was made relative to the working copy.
    cpo::uno::Reference<css::frame::XStorable> xStorable(mxComponent, cpo::uno::UNO_QUERY_THROW);
    xStorable->store();
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    assertXPath(pXmlDoc, "(//text:section-source)[1]", "href", u"../master-chapter.odt");
}

CPPUNIT_TEST_FIXTURE(Test, testLinkedFileReadFromCopy)
{
    // Given a master document whose linked files may not be read where they are, as in a jail:
    comphelper::ScopeGuard g([]() { HostFilter::resetAllowedExtRefPaths(); });
    HostFilter::setAllowedExtRefPaths("");
    createSwGlobalDoc("master-cached-sections.odm");
    SwXTextDocument* pTextDoc = getSwTextDoc();

    // Then the document reports the files by their path relative to it, none of them readable:
    // The id, the section and whether it is readable, of each linked file.
    using LinkedFiles = std::vector<std::tuple<std::string, std::string, bool>>;
    std::function<LinkedFiles()> getLinks = [pTextDoc]() {
        tools::JsonWriter aJson;
        CPPUNIT_ASSERT(pTextDoc->getLinkedFiles(aJson));
        std::stringstream aStream{ std::string(aJson.finishAndGetAsOString()) };
        boost::property_tree::ptree aTree;
        boost::property_tree::read_json(aStream, aTree);
        LinkedFiles aLinks;
        for (const auto& rLink : aTree.get_child("links"))
            aLinks.emplace_back(rLink.second.get<std::string>("id"),
                                rLink.second.get<std::string>("section"),
                                rLink.second.get<bool>("readable"));
        return aLinks;
    };
    LinkedFiles aLinks = getLinks();
    CPPUNIT_ASSERT_EQUAL(size_t(2), aLinks.size());
    CPPUNIT_ASSERT_EQUAL(std::string("master-chapter.odt"), std::get<0>(aLinks[0]));
    CPPUNIT_ASSERT_EQUAL(std::string("Present"), std::get<1>(aLinks[0]));
    CPPUNIT_ASSERT(!std::get<2>(aLinks[0]));
    CPPUNIT_ASSERT_EQUAL(std::string("master-missing-chapter.odt"), std::get<0>(aLinks[1]));
    CPPUNIT_ASSERT(!std::get<2>(aLinks[1]));

    // When the host gives a copy of the missing file, which is read in its place:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), pTextDoc->setLinkedFileSource(u"no-such-file.odt"_ustr,
                                                                      createFileURL(u"master-chapter.odt")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                         pTextDoc->setLinkedFileSource(u"master-missing-chapter.odt"_ustr,
                                                       createFileURL(u"master-chapter.odt")));
    aLinks = getLinks();
    CPPUNIT_ASSERT(!std::get<2>(aLinks[0]));
    CPPUNIT_ASSERT(std::get<2>(aLinks[1]));

    // And the user allows the links to be updated:
    getSwDocShell()->AllowLinkUpdate();
    Scheduler::ProcessEventsToIdle();

    // Then the section of the file with a copy shows the copy, the other keeps its content.
    // Without the fix in place, both files were refused.
    CPPUNIT_ASSERT_EQUAL(u"Cached text of the present chapter"_ustr,
                         getSectionText(u"Present"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text of the chapter file"_ustr, getSectionText(u"Missing"_ustr));
    // And only the section without a copy is a broken link.
    CPPUNIT_ASSERT(!isSectionConnected(u"Present"));
    CPPUNIT_ASSERT(isSectionConnected(u"Missing"));

    // And the document is saved with the link it had, not with the copy.
    save(TestFilter::ODM_GLOBAL);
    xmlDocUniquePtr pXmlDoc = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "(//text:section-source)[2]", "href")
                       .endsWith("/master-missing-chapter.odt"));
}

CPPUNIT_TEST_FIXTURE(Test, testLinkedFileIdOfAbsoluteLink)
{
    // Given a master document:
    createSwGlobalDoc("master-cached-sections.odm");
    SwDocShell* pDocShell = getSwDocShell();

    // Then a file next to it is known by its path relative to it:
    CPPUNIT_ASSERT_EQUAL(u"master-chapter.odt"_ustr,
                         pDocShell->GetLinkedFileId(createFileURL(u"master-chapter.odt")));
    // And a file that shares no folder with it keeps its URL, which a host asks for no copy of.
    // Without the fix in place, the path climbed up to the root from where the document was.
    CPPUNIT_ASSERT_EQUAL(u"file:///chapter.odt"_ustr,
                         pDocShell->GetLinkedFileId(u"file:///chapter.odt"_ustr));
}

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
    auto [nBoxes, nCellSections] = CountTableBoxesAndCellSections(*getSwDoc());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(26), nBoxes);
    CPPUNIT_ASSERT_EQUAL(26, nCellSections);
}

CPPUNIT_TEST_FIXTURE(Test, testTableKeepsCellInsideDeletion)
{
    // Given a document whose tracked deletion starts in one cell and ends two cells later, so the
    // cell between them lies wholly inside the deletion:
    createSwDoc("ofz566727923.doc");

    // Then every box of every table still has a cell section of its own:
    auto [nBoxes, nCellSections] = CountTableBoxesAndCellSections(*getSwDoc());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(25), nBoxes);
    CPPUNIT_ASSERT_EQUAL(25, nCellSections);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
