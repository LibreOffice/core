/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mailmergetestbase.cxx"

namespace
{
class MMTest : public MailMergeTestBase
{
};

DECLARE_SHELL_MAILMERGE_TEST(testMultiPageAnchoredDraws, "multiple-page-anchored-draws.odt", "4_v01.ods", "Tabelle1")
{
    executeMailMerge();

    CPPUNIT_ASSERT(mxSwTextDocument);
    sal_uInt16 nPhysPages = mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), nPhysPages);

    uno::Reference<container::XIndexAccess> xDraws = mxSwTextDocument->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());

    std::set<sal_uInt16> pages;
    uno::Reference<beans::XPropertySet> xPropertySet;

    for (sal_Int32 i = 0; i < xDraws->getCount(); i++)
    {
        xPropertySet.set(xDraws->getByIndex(i), uno::UNO_QUERY);

        text::TextContentAnchorType nAnchorType;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue( UNO_NAME_ANCHOR_TYPE ) >>= nAnchorType);
        CPPUNIT_ASSERT_EQUAL( text::TextContentAnchorType_AT_PAGE, nAnchorType );

        sal_uInt16 nAnchorPageNo = {};
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue( UNO_NAME_ANCHOR_PAGE_NO ) >>= nAnchorPageNo);
        // are all shapes are on different page numbers?
        CPPUNIT_ASSERT(pages.insert(nAnchorPageNo).second);
    }
}

DECLARE_FILE_MAILMERGE_TEST(testMissingDefaultLineColor, "missing-default-line-color.ott", "one-empty-address.ods", "one-empty-address")
{
    executeMailMerge();
    // The document was created by LO version which didn't write out the default value for line color
    // (see XMLGraphicsDefaultStyle::SetDefaults()).
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(5), uno::UNO_QUERY);
    // Lines do not have a line color.
    CPPUNIT_ASSERT( !xPropertySet->getPropertySetInfo()->hasPropertyByName( u"LineColor"_ustr ));
    uno::Reference< lang::XMultiServiceFactory > xFact( mxComponent, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xDefaults( xFact->createInstance( u"com.sun.star.drawing.Defaults"_ustr ), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xDefaults.is());
    uno::Reference< beans::XPropertySetInfo > xInfo( xDefaults->getPropertySetInfo());
    CPPUNIT_ASSERT( xInfo->hasPropertyByName( u"LineColor"_ustr ));
    Color lineColor;
    xDefaults->getPropertyValue( u"LineColor"_ustr ) >>= lineColor;
    // And the default value is black (wasn't copied properly by mailmerge).
    CPPUNIT_ASSERT_EQUAL( COL_BLACK, lineColor );
    // And check that the resulting file has the proper default.
    xmlDocUniquePtr pXmlDoc = parseMailMergeExport( u"styles.xml"_ustr );
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/style:default-style[1]", "family", u"graphic");
    assertXPath(pXmlDoc, "/office:document-styles/office:styles/style:default-style[1]/style:graphic-properties", "stroke-color", u"#000000");
}

DECLARE_FILE_MAILMERGE_TEST(testSimpleMailMerge, "simple-mail-merge.odt", "10-testing-addresses.ods", "testing-addresses")
{
    executeMailMerge();
    for( int doc = 0;
         doc < 10;
         ++doc )
    {
        loadMailMergeDocument( doc );
        CPPUNIT_ASSERT_EQUAL( 1, getPages());
        CPPUNIT_ASSERT_EQUAL( u"Fixed text."_ustr, getRun( getParagraph( 1 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString( "lastname" + OUString::number( doc + 1 )), getRun( getParagraph( 2 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( u"Another fixed text."_ustr, getRun( getParagraph( 3 ), 1 )->getString());
    }
}

DECLARE_FILE_MAILMERGE_TEST(testWriterDataSource, "writer-mail-merge.odt", "10-testing-addresses-writer.odt", "testing-addresses-writer")
{
    // This failed as the .odt data source was mapped to the jdbc: protocol.
    executeMailMerge();
    for (int doc = 0; doc < 10; ++doc)
    {
        loadMailMergeDocument(doc);
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        CPPUNIT_ASSERT_EQUAL(u"Fixed text."_ustr, getRun(getParagraph(1), 1)->getString());
        CPPUNIT_ASSERT_EQUAL(OUString("lastname" + OUString::number(doc + 1)), getRun(getParagraph(2), 1)->getString());
        CPPUNIT_ASSERT_EQUAL(u"Another fixed text."_ustr, getRun(getParagraph(3), 1)->getString());
    }
}

DECLARE_FILE_MAILMERGE_TEST(testWriterMergedDataSource, "writer-merged-mail-merge.odt", "10-testing-addresses-writer-merged.odt", "testing-addresses-writer-merged")
{
    // This failed with com.sun.star.lang.IndexOutOfBoundsException, leading to
    // a crash, as the last row had merged cells in
    // 10-testing-addresses-writer-merged.odt.
    executeMailMerge();
    for (int doc = 0; doc < 10; ++doc)
    {
        loadMailMergeDocument(doc);
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        CPPUNIT_ASSERT_EQUAL(u"Fixed text."_ustr, getRun(getParagraph(1), 1)->getString());
        CPPUNIT_ASSERT_EQUAL(OUString("lastname" + OUString::number(doc + 1)), getRun(getParagraph(2), 1)->getString());
        CPPUNIT_ASSERT_EQUAL(u"Another fixed text."_ustr, getRun(getParagraph(3), 1)->getString());
    }
}

DECLARE_FILE_MAILMERGE_TEST(test2Pages, "simple-mail-merge-2pages.odt", "10-testing-addresses.ods", "testing-addresses")
{
    executeMailMerge();
    for( int doc = 0;
         doc < 10;
         ++doc )
    {
        loadMailMergeDocument( doc );
        OUString lastname = "lastname" + OUString::number( doc + 1 );
        OUString firstname = "firstname" + OUString::number( doc + 1 );
        CPPUNIT_ASSERT_EQUAL( 2, getPages());
        CPPUNIT_ASSERT_EQUAL( u"Fixed text."_ustr, getRun( getParagraph( 1 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( lastname, getRun( getParagraph( 2 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( u"Another fixed text."_ustr, getRun( getParagraph( 3 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString(), getRun( getParagraph( 4 ), 1 )->getString()); // empty para at the end of page 1
        CPPUNIT_ASSERT_EQUAL( u"Second page."_ustr, getRun( getParagraph( 5 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( firstname, getRun( getParagraph( 6 ), 1 )->getString());
        // Also verify the layout.

        xmlDocUniquePtr pXmlDoc = parseLayoutDump(static_cast<SfxBaseModel*>(mxSwTextDocument.get()));
        assertXPath(pXmlDoc, "/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", lastname);
        assertXPathContent(pXmlDoc, "/root/page[1]/body/txt[1]", u"Fixed text.");
        assertXPathContent(pXmlDoc, "/root/page[1]/body/txt[4]", u"");
        assertXPathContent(pXmlDoc, "/root/page[2]/body/txt[1]", u"Second page.");
        assertXPath(pXmlDoc,  "/root/page[2]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion", "expand", firstname);
    }
}

DECLARE_SHELL_MAILMERGE_TEST(testPageBoundariesSimpleMailMerge, "simple-mail-merge.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // This is like the test above, but this one uses the create-single-document-containing-everything-generated approach,
    // and verifies that boundaries of the generated sub-documents are correct inside that document.
    // These boundaries are done using "documentStartPageNumber<number>" UNO bookmarks (see also
    // documentStartPageNumber() ).
    executeMailMerge();
    // Here getPages() works on the source document, so get pages of the resulting one.
    CPPUNIT_ASSERT(mxSwTextDocument);
    CPPUNIT_ASSERT_EQUAL( sal_uInt16( 19 ), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum()); // 10 pages, but each sub-document starts on odd page number
    for( int doc = 0;
         doc < 10;
         ++doc )
    {
        CPPUNIT_ASSERT_EQUAL( doc * 2 + 1, documentStartPageNumber( doc ));
    }
}

DECLARE_SHELL_MAILMERGE_TEST(testPageBoundaries2Pages, "simple-mail-merge-2pages.odt", "10-testing-addresses.ods", "testing-addresses")
{
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    CPPUNIT_ASSERT_EQUAL( sal_uInt16( 20 ), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum()); // 20 pages, each sub-document starts on odd page number
    for( int doc = 0;
         doc < 10;
         ++doc )
    {
        CPPUNIT_ASSERT_EQUAL( doc * 2 + 1, documentStartPageNumber( doc ));
    }
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf89214, "tdf89214.odt", "10-testing-addresses.ods", "testing-addresses")
{
    executeMailMerge();

    uno::Reference<text::XTextRange> xParagraph(getParagraphOrTable(3, mxSwTextDocument->getText()), uno::UNO_QUERY);
    // Make sure that we assert the right paragraph.
    CPPUNIT_ASSERT_EQUAL(u"a"_ustr, xParagraph->getString());
    // This paragraph had a bullet numbering, make sure that the list id is not empty.
    CPPUNIT_ASSERT(!getProperty<OUString>(xParagraph, u"ListId"_ustr).isEmpty());
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf90230, "empty.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // MM of an empty document caused an assertion in the SwContentIndexReg dtor.
    executeMailMerge();
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf92623, "tdf92623.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // Copying bookmarks for MM was broken because of the StartOfContent node copy
    // copied marks were off by one
    executeMailMerge();

    IDocumentMarkAccess const *pIDMA = getSwDoc()->getIDocumentMarkAccess();
    // There is just one mark...
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pIDMA->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pIDMA->getBookmarksCount());
    auto mark = pIDMA->getAllMarksBegin();
    // and it's a TEXT_FIELDMARK
    CPPUNIT_ASSERT_EQUAL( sal_Int32(IDocumentMarkAccess::MarkType::TEXT_FIELDMARK),
                            sal_Int32(IDocumentMarkAccess::GetType( **mark )) );
    SwNodeOffset src_pos = (*mark)->GetMarkPos().GetNodeIndex();

    // Get the size of the document in nodes
    SwDoc *doc = getSwDoc();
    SwNodeOffset size = doc->GetNodes().GetEndOfContent().GetIndex() - doc->GetNodes().GetEndOfExtras().GetIndex();
    CPPUNIT_ASSERT_EQUAL( SwNodeOffset(13), size );
    size -= SwNodeOffset(2); // For common start and end nodes

    // Iterate over all field marks in the target document and check that they
    // are positioned at a multitude of the document size
    CPPUNIT_ASSERT(mxSwTextDocument);
    pIDMA = mxSwTextDocument->GetDocShell()->GetDoc()->getIDocumentMarkAccess();
    // The target document has the duplicated amount of bookmarks
    // as the helping uno bookmark from the mail merge is left in the doc
    // TODO should be fixed!
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), pIDMA->getAllMarksCount());
    std::set<SwNodeOffset> pages;
    sal_Int32 countFieldMarks = 0;
    for( mark = pIDMA->getAllMarksBegin(); mark != pIDMA->getAllMarksEnd(); ++mark )
    {
        IDocumentMarkAccess::MarkType markType = IDocumentMarkAccess::GetType( **mark );
        if( markType == IDocumentMarkAccess::MarkType::TEXT_FIELDMARK )
        {
            SwNodeOffset pos = (*mark)->GetMarkPos().GetNodeIndex() - src_pos;
            CPPUNIT_ASSERT_EQUAL(SwNodeOffset(0), pos % size);
            CPPUNIT_ASSERT(pages.insert(pos).second);
            countFieldMarks++;
        }
        else // see previous TODO
            CPPUNIT_ASSERT_EQUAL( sal_Int32(IDocumentMarkAccess::MarkType::UNO_BOOKMARK), sal_Int32(markType) );
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), countFieldMarks);
}

DECLARE_SHELL_MAILMERGE_TEST(testBookmarkCondition, "bookmarkcondition.fodt", "bookmarkcondition.ods", "data")
{
    executeMailMerge();

    xmlDocUniquePtr pXmlDoc = parseLayoutDump(static_cast<SfxBaseModel*>(mxSwTextDocument.get()));
    // check that conditions on sections and bookmarks are evaluated the same
    assertXPath(pXmlDoc, "/root/page", 7);
    assertXPath(pXmlDoc, "/root/page[1]/body/section", 1);
    assertXPath(pXmlDoc, "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout", "portion", u"In den Bergen war es anstrengend.");
    assertXPath(pXmlDoc, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout", "portion", u"Mein Urlaub war anstrengend . ");
    assertXPath(pXmlDoc, "/root/page[3]/body/section", 1);
    assertXPath(pXmlDoc, "/root/page[3]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout", "portion", u"In Barcelona war es schön.");
    assertXPath(pXmlDoc, "/root/page[3]/body/txt[5]/SwParaPortion/SwLineLayout", "portion", u"Mein Urlaub war schön . ");
    assertXPath(pXmlDoc, "/root/page[5]/body/section", 1);
    assertXPath(pXmlDoc, "/root/page[5]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout", "portion", u"In Paris war es erlebnisreich.");
    assertXPath(pXmlDoc, "/root/page[5]/body/txt[5]/SwParaPortion/SwLineLayout", "portion", u"Mein Urlaub war erlebnisreich . ");
    assertXPath(pXmlDoc, "/root/page[7]/body/section", 3);
    assertXPath(pXmlDoc, "/root/page[7]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout", "portion", u"In den Bergen war es anstrengend.");
    assertXPath(pXmlDoc, "/root/page[7]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout", "portion", u"In Barcelona war es schön.");
    assertXPath(pXmlDoc, "/root/page[7]/body/section[3]/txt[1]/SwParaPortion/SwLineLayout", "portion", u"In Paris war es erlebnisreich.");
    assertXPath(pXmlDoc, "/root/page[7]/body/txt[5]/SwParaPortion/SwLineLayout", "portion", u"Mein Urlaub war anstrengend schön erlebnisreich . ");
}

DECLARE_SHELL_MAILMERGE_TEST_SELECTION(testTdf95292, "linked-labels.odt", "10-testing-addresses.ods", "testing-addresses", 5)
{
    // A document with two labels merged with 5 datasets should result in three pages
    executeMailMerge();

    SwWrtShell *pWrtShell = getSwDocShell()->GetWrtShell();
    CPPUNIT_ASSERT( pWrtShell->IsLabelDoc() );

    CPPUNIT_ASSERT( mxSwTextDocument );
    pWrtShell = mxSwTextDocument->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT( !pWrtShell->IsLabelDoc() );
    CPPUNIT_ASSERT_EQUAL( sal_uInt16( 5 ), pWrtShell->GetPhyPageNum() );
}

DECLARE_SHELL_MAILMERGE_TEST(test_sections_first_last, "sections_first_last.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // A document with a leading, middle and trailing section
    // Originally we were losing the trailing section during merge
    executeMailMerge();

    // Get the size of the document in nodes
    SwDoc *pDoc = getSwDoc();
    SwNodeOffset nSize = pDoc->GetNodes().GetEndOfContent().GetIndex() - pDoc->GetNodes().GetEndOfExtras().GetIndex();
    nSize -= SwNodeOffset(2); // The common start and end node
    CPPUNIT_ASSERT_EQUAL( SwNodeOffset(13), nSize );

    CPPUNIT_ASSERT(mxSwTextDocument);

    SwDoc *pDocMM = mxSwTextDocument->GetDocShell()->GetDoc();
    SwNodeOffset nSizeMM = pDocMM->GetNodes().GetEndOfContent().GetIndex() - pDocMM->GetNodes().GetEndOfExtras().GetIndex();
    nSizeMM -= SwNodeOffset(2);
    CPPUNIT_ASSERT_EQUAL( SwNodeOffset(10) * nSize, nSizeMM );

    CPPUNIT_ASSERT_EQUAL( sal_uInt16(19), pDocMM->GetDocShell()->GetWrtShell()->GetPhyPageNum() );

    // All even pages should be empty, all sub-documents have two pages
    const SwRootFrame* pLayout = pDocMM->getIDocumentLayoutAccess().GetCurrentLayout();
    const SwPageFrame* pPageFrm = static_cast<const SwPageFrame*>( pLayout->Lower() );
    while ( pPageFrm )
    {
        bool bOdd = (1 == (pPageFrm->GetPhyPageNum() % 2));
        CPPUNIT_ASSERT_EQUAL( !bOdd, pPageFrm->IsEmptyPage() );
        CPPUNIT_ASSERT_EQUAL( sal_uInt16( bOdd ? 1 : 2 ), pPageFrm->GetVirtPageNum() );
        pPageFrm = static_cast<const SwPageFrame*>( pPageFrm->GetNext() );
    }
}

DECLARE_FILE_MAILMERGE_TEST_COLUMN(testDirMailMerge, "simple-mail-merge.odt", "10-testing-addresses.ods", "testing-addresses", "Filename")
{
    executeMailMerge();
    for( int doc = 1;
         doc <= 10;
         ++doc )
    {
        OUString filename = "sub/lastname" + OUString::number( doc )
                          + " firstname" + OUString::number( doc ) + ".odt";
        loadMailMergeDocument( filename );
        CPPUNIT_ASSERT_EQUAL( 1, getPages());
        CPPUNIT_ASSERT_EQUAL( u"Fixed text."_ustr, getRun( getParagraph( 1 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString( "lastname" + OUString::number( doc )), getRun( getParagraph( 2 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( u"Another fixed text."_ustr, getRun( getParagraph( 3 ), 1 )->getString());
    }
}

DECLARE_FILE_MAILMERGE_TEST(testTdf102010, "empty.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // Create "correct" automatic filename for non-user-supplied-prefix
    for (auto aNamedValueIter = mMMargs.begin(); aNamedValueIter != mMMargs.end();)
    {
        if ( aNamedValueIter->Name == UNO_NAME_FILE_NAME_PREFIX )
            aNamedValueIter = mMMargs.erase( aNamedValueIter );
        else
        {
            std::cout << aNamedValueIter->Name << ": " << aNamedValueIter->Value << std::endl;
            ++aNamedValueIter;
        }
    }
    mMMargs.emplace_back( UNO_NAME_SAVE_AS_SINGLE_FILE, uno::Any( true ) );

    // Generate correct mail merge result filename
    executeMailMerge();
    // Don't overwrite previous result
    executeMailMerge( true );
    loadMailMergeDocument( 1 );
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf118113, "tdf118113.odt", "tdf118113.ods", "testing-addresses")
{
    executeMailMerge();

    // The document contains a text box anchored to the page and a conditionally hidden
    // section that is only shown for one of the 4 recipients, namely the 3rd record.
    // In case the hidden section is shown, the page count is 3 for a single data entry, otherwise 1.
    // Previously, the page number was calculated incorrectly which led to the
    // text box being anchored to the wrong page.

    CPPUNIT_ASSERT(mxSwTextDocument);
    // 3 documents with 1 page size each + 1 document with 3 pages
    // + an additional page after each of the first 3 documents to make
    // sure that each document starts on an odd page number
    sal_uInt16 nPhysPages = mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(9), nPhysPages);

    // verify that there is a text box for each data record
    uno::Reference<container::XIndexAccess> xDraws = mxSwTextDocument->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDraws->getCount());

    // verify the text box for each data record is anchored to the first page of the given data record's pages
    std::vector<sal_uInt16> expectedPageNumbers {1, 3, 5, 9};
    uno::Reference<beans::XPropertySet> xPropertySet;
    for (sal_Int32 i = 0; i < xDraws->getCount(); i++)
    {
        xPropertySet.set(xDraws->getByIndex(i), uno::UNO_QUERY);

        text::TextContentAnchorType nAnchorType;
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue( UNO_NAME_ANCHOR_TYPE ) >>= nAnchorType);
        CPPUNIT_ASSERT_EQUAL( text::TextContentAnchorType_AT_PAGE, nAnchorType );

        sal_uInt16 nAnchorPageNo = {};
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue( UNO_NAME_ANCHOR_PAGE_NO ) >>= nAnchorPageNo);

        CPPUNIT_ASSERT_EQUAL(expectedPageNumbers.at(i), nAnchorPageNo);
    }
}


constexpr OUString EmptyValuesLegacyData[] = {
    // Doc 1 [ Title: ""; First Name: "firstname1"; Last Name: "lastname1" ]
    u"Heading"_ustr,
    // Title: [Title]
    u"Title: "_ustr,
    // First Name: [First Name]
    u"First Name: firstname1"_ustr,
    // Last Name: [Last Name]
    u"Last Name: lastname1"_ustr,
    // Title: [Title] First Name: [First Name]
    u"Title:  First Name: firstname1"_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name: firstname1 Last Name: lastname1"_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title:  First Name: firstname1 Last Name: lastname1"_ustr,
    u"Trailing text"_ustr,

    // Doc 2 [ Title: "title2"; First Name: ""; Last Name: "lastname2" ]
    u"Heading"_ustr,
    // Title: [Title]
    u"Title: title2"_ustr,
    // First Name: [First Name]
    u"First Name: "_ustr,
    // Last Name: [Last Name]
    u"Last Name: lastname2"_ustr,
    // Title: [Title] First Name: [First Name]
    u"Title: title2 First Name: "_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name:  Last Name: lastname2"_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title: title2 First Name:  Last Name: lastname2"_ustr,
    u"Trailing text"_ustr,

    // Doc 3 [ Title: "title3"; First Name: "firstname3"; Last Name: "" ]
    u"Heading"_ustr,
    // Title: [Title]
    u"Title: title3"_ustr,
    // First Name: [First Name]
    u"First Name: firstname3"_ustr,
    // Last Name: [Last Name]
    u"Last Name: "_ustr,
    // Title: [Title] First Name: [First Name]
    u"Title: title3 First Name: firstname3"_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name: firstname3 Last Name: "_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title: title3 First Name: firstname3 Last Name: "_ustr,
    u"Trailing text"_ustr,

    // Doc 4 [ Title: ""; First Name: ""; Last Name: "lastname4" ]
    u"Heading"_ustr,
    // Title: [Title]
    u"Title: "_ustr,
    // First Name: [First Name]
    u"First Name: "_ustr,
    // Last Name: [Last Name]
    u"Last Name: lastname4"_ustr,
    // Title: [Title] First Name: [First Name]
    u"Title:  First Name: "_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name:  Last Name: lastname4"_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title:  First Name:  Last Name: lastname4"_ustr,
    u"Trailing text"_ustr,

    // Doc 5 [ Title: "title5"; First Name: ""; Last Name: "" ]
    u"Heading"_ustr,
    // Title: [Title]
    u"Title: title5"_ustr,
    // First Name: [First Name]
    u"First Name: "_ustr,
    // Last Name: [Last Name]
    u"Last Name: "_ustr,
    // Title: [Title] First Name: [First Name]
    u"Title: title5 First Name: "_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name:  Last Name: "_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title: title5 First Name:  Last Name: "_ustr,
    u"Trailing text"_ustr,
};
constexpr OUString EmptyValuesNewData[] = {
    // Doc 1 [ Title: ""; First Name: "firstname1"; Last Name: "lastname1" ]
    u"Heading"_ustr,
    // Title: [Title]
    // First Name: [First Name]
    u"First Name: firstname1"_ustr,
    // Last Name: [Last Name]
    u"Last Name: lastname1"_ustr,
    // Title: [Title] First Name: [First Name]
    u"Title:  First Name: firstname1"_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name: firstname1 Last Name: lastname1"_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title:  First Name: firstname1 Last Name: lastname1"_ustr,
    u"Trailing text"_ustr,

    // Doc 2 [ Title: "title2"; First Name: ""; Last Name: "lastname2" ]
    u"Heading"_ustr,
    // Title: [Title]
    u"Title: title2"_ustr,
    // First Name: [First Name]
    // Last Name: [Last Name]
    u"Last Name: lastname2"_ustr,
    // Title: [Title] First Name: [First Name]
    u"Title: title2 First Name: "_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name:  Last Name: lastname2"_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title: title2 First Name:  Last Name: lastname2"_ustr,
    u"Trailing text"_ustr,

    // Doc 3 [ Title: "title3"; First Name: "firstname3"; Last Name: "" ]
    u"Heading"_ustr,
    // Title: [Title]
    u"Title: title3"_ustr,
    // First Name: [First Name]
    u"First Name: firstname3"_ustr,
    // Last Name: [Last Name]
    // Title: [Title] First Name: [First Name]
    u"Title: title3 First Name: firstname3"_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name: firstname3 Last Name: "_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title: title3 First Name: firstname3 Last Name: "_ustr,
    u"Trailing text"_ustr,

    // Doc 4 [ Title: ""; First Name: ""; Last Name: "lastname4" ]
    u"Heading"_ustr,
    // Title: [Title]
    // First Name: [First Name]
    // Last Name: [Last Name]
    u"Last Name: lastname4"_ustr,
    // Title: [Title] First Name: [First Name]
    // First Name: [First Name] Last Name: [Last Name]
    u"First Name:  Last Name: lastname4"_ustr,
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title:  First Name:  Last Name: lastname4"_ustr,
    u"Trailing text"_ustr,

    // Doc 5 [ Title: "title5"; First Name: ""; Last Name: "" ]
    u"Heading"_ustr,
    // Title: [Title]
    u"Title: title5"_ustr,
    // First Name: [First Name]
    // Last Name: [Last Name]
    // Title: [Title] First Name: [First Name]
    u"Title: title5 First Name: "_ustr,
    // First Name: [First Name] Last Name: [Last Name]
    // Title: [Title] First Name: [First Name] Last Name: [Last Name]
    u"Title: title5 First Name:  Last Name: "_ustr,
    u"Trailing text"_ustr,
};


// The following four tests (testEmptyValuesLegacyODT, testEmptyValuesNewODT, testEmptyValuesLegacyFODT, testEmptyValuesNewFODT)
// check that for native documents without "EmptyDbFieldHidesPara" compatibility option, all paragraphs are exported visible,
// while for documents with the option enabled, the paragraphs with all Database fields having empty values are removed.

DECLARE_SHELL_MAILMERGE_TEST(testEmptyValuesLegacyODT, "tdf35798-legacy.odt", "5-with-blanks.ods",
                             "names")
{
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    for (size_t i = 0; i < std::size(EmptyValuesLegacyData); ++i)
    {
        auto xPara = getParagraphOfText(i + 1, mxSwTextDocument->getText());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("paragraph " + OString::number(i + 1)).getStr(),
                                     EmptyValuesLegacyData[i], xPara->getString());
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(std::size(EmptyValuesLegacyData)),
                         getParagraphs(mxSwTextDocument->getText()));
}

DECLARE_SHELL_MAILMERGE_TEST(testEmptyValuesNewODT, "tdf35798-new.odt", "5-with-blanks.ods",
                             "names")
{
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    for (size_t i = 0; i < std::size(EmptyValuesNewData); ++i)
    {
        auto xPara = getParagraphOfText(i + 1, mxSwTextDocument->getText());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("paragraph " + OString::number(i + 1)).getStr(),
                                     EmptyValuesNewData[i], xPara->getString());
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(std::size(EmptyValuesNewData)),
                         getParagraphs(mxSwTextDocument->getText()));
}

DECLARE_SHELL_MAILMERGE_TEST(testEmptyValuesLegacyFODT, "tdf35798-legacy.fodt", "5-with-blanks.ods",
                             "names")
{
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    for (size_t i = 0; i < std::size(EmptyValuesLegacyData); ++i)
    {
        auto xPara = getParagraphOfText(i + 1, mxSwTextDocument->getText());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("paragraph " + OString::number(i + 1)).getStr(),
                                     EmptyValuesLegacyData[i], xPara->getString());
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(std::size(EmptyValuesLegacyData)),
                         getParagraphs(mxSwTextDocument->getText()));
}

DECLARE_SHELL_MAILMERGE_TEST(testEmptyValuesNewFODT, "tdf35798-new.fodt", "5-with-blanks.ods",
                             "names")
{
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    for (size_t i = 0; i < std::size(EmptyValuesNewData); ++i)
    {
        auto xPara = getParagraphOfText(i + 1, mxSwTextDocument->getText());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("paragraph " + OString::number(i + 1)).getStr(),
                                     EmptyValuesNewData[i], xPara->getString());
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(std::size(EmptyValuesNewData)),
                         getParagraphs(mxSwTextDocument->getText()));
}

DECLARE_SHELL_MAILMERGE_TEST(testEmptyValuesDOCX, "tdf35798.docx", "5-with-blanks.ods",
                             "names")
{
    // DOCX - and any other Word format - must use the "hide empty DB fields" compatibility option.
    // The feature was introduced to match Word's existing functionality in the first place.
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    for (size_t i = 0; i < std::size(EmptyValuesNewData); ++i)
    {
        auto xPara = getParagraphOfText(i + 1, mxSwTextDocument->getText());
        CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("paragraph " + OString::number(i + 1)).getStr(),
                                     EmptyValuesNewData[i], xPara->getString());
    }
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(std::size(EmptyValuesNewData)),
                         getParagraphs(mxSwTextDocument->getText()));
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf118845, "tdf118845.fodt", "4_v01.ods", "Tabelle1")
{
    executeMailMerge();

    // Both male and female greetings were shown, thus each page had 3 paragraphs

    CPPUNIT_ASSERT(mxSwTextDocument);
    sal_uInt16 nPhysPages = mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(7), nPhysPages); // 4 pages, each odd, and 3 blanks

    CPPUNIT_ASSERT_EQUAL(8, getParagraphs(mxSwTextDocument->getText()));

    uno::Reference<text::XTextRange> xParagraph(getParagraphOrTable(1, mxSwTextDocument->getText()),
                                                uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Dear Mrs. Mustermann1,"_ustr, xParagraph->getString());

    xParagraph.set(getParagraphOrTable(2, mxSwTextDocument->getText()), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xParagraph->getString());

    xParagraph.set(getParagraphOrTable(3, mxSwTextDocument->getText()), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Dear Mr. Mustermann2,"_ustr, xParagraph->getString());

    xParagraph.set(getParagraphOrTable(4, mxSwTextDocument->getText()), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xParagraph->getString());

    xParagraph.set(getParagraphOrTable(5, mxSwTextDocument->getText()), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Dear Mrs. Mustermann3,"_ustr, xParagraph->getString());

    xParagraph.set(getParagraphOrTable(6, mxSwTextDocument->getText()), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xParagraph->getString());

    xParagraph.set(getParagraphOrTable(7, mxSwTextDocument->getText()), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Dear Mr. Mustermann4,"_ustr, xParagraph->getString());

    xParagraph.set(getParagraphOrTable(8, mxSwTextDocument->getText()), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, xParagraph->getString());
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf62364, "tdf62364.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // prepare unit test and run
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    CPPUNIT_ASSERT_EQUAL( sal_uInt16( 19 ), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum()); // 10 pages, but each sub-document starts on odd page number

    // check: each page (one page is one sub doc) has 4 paragraphs:
    // - 1st and 2nd are regular paragraphs
    // - 3rd and 4th are inside list
    const bool nodeInList[4] = { false, false, true, true };

    const auto & rNodes = mxSwTextDocument->GetDocShell()->GetDoc()->GetNodes();
    for (int pageIndex=0; pageIndex<10; pageIndex++)
    {
        for (int nodeIndex = 0; nodeIndex<4; nodeIndex++)
        {
            SwNode* aNode = rNodes[SwNodeOffset(9 + pageIndex * 4 + nodeIndex)];
            CPPUNIT_ASSERT_EQUAL(true, aNode->IsTextNode());

            const SwTextNode* pTextNode = aNode->GetTextNode();
            CPPUNIT_ASSERT(pTextNode);
            CPPUNIT_ASSERT_EQUAL(nodeInList[nodeIndex], pTextNode->GetSwAttrSet().HasItem(RES_PARATR_LIST_ID));
        }
    }
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
