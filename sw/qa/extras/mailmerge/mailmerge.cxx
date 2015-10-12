/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <set>

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/MailMergeType.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <olmenu.hxx>
#include <cmdid.h>

/**
 * Maps database URIs to the registered database names for quick lookups
 */
typedef std::map<OUString, OUString> DBuriMap;
DBuriMap aDBuriMap;

class MMTest : public SwModelTestBase
{
public:
    MMTest();

    virtual void tearDown() override
    {
        if (mxMMComponent.is())
        {
            if (mnCurOutputType == text::MailMergeType::SHELL)
            {
                SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxMMComponent.get());
                CPPUNIT_ASSERT(pTextDoc);
                pTextDoc->GetDocShell()->DoClose();
            }
            else
                mxMMComponent->dispose();
        }
        SwModelTestBase::tearDown();
    }

    /**
     * Helper func used by each unit test to test the 'mail merge' code.
     *
     * Registers the data source, loads the original file as reference,
     * initializes the mail merge job and its default argument sequence.
     *
     * The 'verify' method actually has to execute the mail merge by
     * calling executeMailMerge() after modifying the job arguments.
     */
    void executeMailMergeTest(const char* filename, const char* datasource, const char* tablename, bool file)
    {
        header();
        preTest(filename);
        load(mpTestDocumentPath, filename);

        utl::TempFile aTempDir(nullptr, true);
        const OUString aWorkDir = aTempDir.GetURL();
        const OUString aURI( getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(datasource) );
        OUString aDBName = registerDBsource( aURI, aWorkDir );
        initMailMergeJobAndArgs( filename, tablename, aDBName, "LOMM_", aWorkDir, file );

        postTest(filename);
        verify();
        finish();

        ::utl::removeTree(aWorkDir);
        mnCurOutputType = 0;
    }

    OUString registerDBsource( const OUString &aURI, const OUString &aWorkDir )
    {
        OUString aDBName;
        DBuriMap::const_iterator pos = aDBuriMap.find( aURI );
        if (pos == aDBuriMap.end())
        {
            aDBName = SwDBManager::LoadAndRegisterDataSource( aURI, NULL, &aWorkDir );
            aDBuriMap.insert( std::pair< OUString, OUString >( aURI, aDBName ) );
            std::cout << "New datasource name: '" << aDBName << "'" << std::endl;
        }
        else
        {
            aDBName = pos->second;
            std::cout << "Old datasource name: '" << aDBName << "'" << std::endl;
        }
        CPPUNIT_ASSERT(!aDBName.isEmpty());
        return aDBName;
    }

    void initMailMergeJobAndArgs( const char* filename, const char* tablename, const OUString &aDBName,
                                          const OUString &aPrefix, const OUString &aWorkDir, bool file )
    {
        uno::Reference< task::XJob > xJob( getMultiServiceFactory()->createInstance( "com.sun.star.text.MailMerge" ), uno::UNO_QUERY_THROW );
        mxJob.set( xJob );

        int seq_id = 5;
        if (tablename) seq_id += 2;
        mSeqMailMergeArgs.realloc( seq_id );

        seq_id = 0;
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUString( UNO_NAME_OUTPUT_TYPE ), uno::Any( file ? text::MailMergeType::FILE : text::MailMergeType::SHELL ) );
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUString( UNO_NAME_DOCUMENT_URL ), uno::Any(
                                        ( OUString(getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(filename)) ) ) );
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUString( UNO_NAME_DATA_SOURCE_NAME ), uno::Any( aDBName ) );
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUString( UNO_NAME_OUTPUT_URL ), uno::Any( aWorkDir ) );
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUString( UNO_NAME_FILE_NAME_PREFIX ), uno::Any( aPrefix ));
        if (tablename)
        {
            mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUString( UNO_NAME_DAD_COMMAND_TYPE ), uno::Any( sdb::CommandType::TABLE ) );
            mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUString( UNO_NAME_DAD_COMMAND ), uno::Any( OUString::createFromAscii(tablename) ) );
        }
    }

    void executeMailMerge()
    {
        uno::Any res = mxJob->execute( mSeqMailMergeArgs );

        const beans::NamedValue *pArguments = mSeqMailMergeArgs.getConstArray();
        bool bOk = true;
        sal_Int32 nArgs = mSeqMailMergeArgs.getLength();

        for (sal_Int32 i = 0; i < nArgs; ++i) {
            const OUString &rName  = pArguments[i].Name;
            const uno::Any &rValue = pArguments[i].Value;

            // all error checking was already done by the MM job execution
            if (rName == UNO_NAME_OUTPUT_URL)
                bOk &= rValue >>= mailMergeOutputURL;
            else if (rName == UNO_NAME_FILE_NAME_PREFIX)
                bOk &= rValue >>= mailMergeOutputPrefix;
            else if (rName == UNO_NAME_OUTPUT_TYPE)
                bOk &= rValue >>= mnCurOutputType;
        }

        CPPUNIT_ASSERT(bOk);

        if (mnCurOutputType == text::MailMergeType::SHELL)
        {
            CPPUNIT_ASSERT(res >>= mxMMComponent);
            CPPUNIT_ASSERT(mxMMComponent.is());
        }
        else
        {
            CPPUNIT_ASSERT(res == true);
            loadMailMergeDocument( 0 );
        }
    }

    /**
     * Like parseExport(), but for given mail merge document.
     */
    xmlDocPtr parseMailMergeExport(int number, const OUString& rStreamName = OUString("word/document.xml"))
    {
        if (mnCurOutputType != text::MailMergeType::FILE)
            return 0;

        OUString name = mailMergeOutputPrefix + OUString::number( number ) + ".odt";
        return parseExportInternal( mailMergeOutputURL + "/" + name, rStreamName );
    }

    /**
     Loads number-th document from mail merge. Requires file output from mail merge.
    */
    void loadMailMergeDocument( int number )
    {
        assert( mnCurOutputType == text::MailMergeType::FILE );
        if (mxComponent.is())
            mxComponent->dispose();
        OUString name = mailMergeOutputPrefix + OUString::number( number ) + ".odt";
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        std::cout << name << ",";
        mnStartTime = osl_getGlobalTimer();
        mxComponent = loadFromDesktop(mailMergeOutputURL + "/" + name, "com.sun.star.text.TextDocument");
        CPPUNIT_ASSERT( mxComponent.is());
        OString name2 = OUStringToOString( name, RTL_TEXTENCODING_UTF8 );
        discardDumpedLayout();
        if (mustCalcLayoutOf(name2.getStr()))
            calcLayout();
    }

protected:
    // Returns page number of the first page of a MM document inside the large MM document (used in the SHELL case).
    int documentStartPageNumber( int document ) const;

    uno::Reference< com::sun::star::task::XJob > mxJob;
    uno::Sequence< beans::NamedValue > mSeqMailMergeArgs;
    OUString mailMergeOutputURL;
    OUString mailMergeOutputPrefix;
    sal_Int16 mnCurOutputType;
    uno::Reference< lang::XComponent > mxMMComponent;
};

#define DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, file, BaseClass) \
    class TestName : public BaseClass { \
    protected: \
        virtual OUString getTestName() override { return OUString(#TestName); } \
    public: \
        CPPUNIT_TEST_SUITE(TestName); \
        CPPUNIT_TEST(MailMerge); \
        CPPUNIT_TEST_SUITE_END(); \
    \
        void MailMerge() { \
            executeMailMergeTest(filename, datasource, tablename, file); \
        } \
        void verify() override; \
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

// Will generate the resulting document in mxMMDocument.
#define DECLARE_SHELL_MAILMERGE_TEST(TestName, filename, datasource, tablename) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, false, MMTest)

// Will generate documents as files, use loadMailMergeDocument().
#define DECLARE_FILE_MAILMERGE_TEST(TestName, filename, datasource, tablename) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, true, MMTest)

int MMTest::documentStartPageNumber( int document ) const
{   // See SwMailMergeOutputPage::documentStartPageNumber() .
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxMMComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* shell = pTextDoc->GetDocShell()->GetWrtShell();
    IDocumentMarkAccess* marks = shell->GetDoc()->getIDocumentMarkAccess();
    // Unfortunately, the pages are marked using UNO bookmarks, which have internals names, so they cannot be referred to by their names.
    // Assume that there are no other UNO bookmarks than the ones used by mail merge, and that they are in the sorted order.
    IDocumentMarkAccess::const_iterator_t mark;
    int pos = 0;
    for( mark = marks->getAllMarksBegin(); mark != marks->getAllMarksEnd() && pos < document; ++mark )
    {
        if( IDocumentMarkAccess::GetType( **mark ) == IDocumentMarkAccess::MarkType::UNO_BOOKMARK )
            ++pos;
    }
    CPPUNIT_ASSERT( pos == document );
    sal_uInt16 page, dummy;
    shell->Push();
    shell->GotoMark( mark->get());
    shell->GetPageNum( page, dummy );
    shell->Pop(false);
    return page;
}
MMTest::MMTest()
    : SwModelTestBase("/sw/qa/extras/mailmerge/data/", "writer8")
    , mnCurOutputType(0)
{
}

DECLARE_SHELL_MAILMERGE_TEST(testMultiPageAnchoredDraws, "multiple-page-anchored-draws.odt", "4_v01.ods", "Tabelle1")
{
    executeMailMerge();

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxMMComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    sal_uInt16 nPhysPages = pTextDoc->GetDocShell()->GetWrtShell()->GetPhyPageNum();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), nPhysPages);

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxMMComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());

    std::set<sal_uInt16> pages;
    uno::Reference<beans::XPropertySet> xPropertySet;

    for (sal_Int32 i = 0; i < xDraws->getCount(); i++)
    {
        text::TextContentAnchorType nAnchorType;
        sal_uInt16 nAnchorPageNo;
        xPropertySet.set(xDraws->getByIndex(i), uno::UNO_QUERY);

        xPropertySet->getPropertyValue( UNO_NAME_ANCHOR_TYPE ) >>= nAnchorType;
        CPPUNIT_ASSERT_EQUAL( text::TextContentAnchorType_AT_PAGE, nAnchorType );

        xPropertySet->getPropertyValue( UNO_NAME_ANCHOR_PAGE_NO ) >>= nAnchorPageNo;
        // are all shapes are on different page numbers?
        CPPUNIT_ASSERT(pages.insert(nAnchorPageNo).second);
    }
}

DECLARE_FILE_MAILMERGE_TEST(testMissingDefaultLineColor, "missing-default-line-color.ott", "one-empty-address.ods", "one-empty-address")
{
    executeMailMerge();
    // The document was created by LO version which didn't write out the default value for line color
    // (see XMLGraphicsDefaultStyle::SetDefaults()).
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    // Lines do not have a line color.
    CPPUNIT_ASSERT( !xPropertySet->getPropertySetInfo()->hasPropertyByName( "LineColor" ));
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    uno::Reference< lang::XMultiServiceFactory > xFact( mxComponent, uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xDefaults( xFact->createInstance( "com.sun.star.drawing.Defaults" ), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xDefaults.is());
    uno::Reference< beans::XPropertySetInfo > xInfo( xDefaults->getPropertySetInfo());
    CPPUNIT_ASSERT( xInfo->hasPropertyByName( "LineColor" ));
    sal_uInt32 lineColor;
    xDefaults->getPropertyValue( "LineColor" ) >>= lineColor;
    // And the default value is black (wasn't copied properly by mailmerge).
    CPPUNIT_ASSERT_EQUAL( COL_BLACK, lineColor );
    // And check that the resulting file has the proper default.
    xmlDocPtr pXmlDoc = parseMailMergeExport( 0, "styles.xml" );
    CPPUNIT_ASSERT_EQUAL( OUString( "graphic" ), getXPath(pXmlDoc, "/office:document-styles/office:styles/style:default-style[1]", "family"));
    CPPUNIT_ASSERT_EQUAL( OUString( "#000000" ), getXPath(pXmlDoc, "/office:document-styles/office:styles/style:default-style[1]/style:graphic-properties", "stroke-color"));
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
        CPPUNIT_ASSERT_EQUAL( OUString( "Fixed text." ), getRun( getParagraph( 1 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString( "lastname" + OUString::number( doc + 1 )), getRun( getParagraph( 2 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString( "Another fixed text." ), getRun( getParagraph( 3 ), 1 )->getString());
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
        CPPUNIT_ASSERT_EQUAL( OUString( "Fixed text." ), getRun( getParagraph( 1 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( lastname, getRun( getParagraph( 2 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString( "Another fixed text." ), getRun( getParagraph( 3 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 4 ), 1 )->getString()); // empty para at the end of page 1
        CPPUNIT_ASSERT_EQUAL( OUString( "Second page." ), getRun( getParagraph( 5 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( firstname, getRun( getParagraph( 6 ), 1 )->getString());
        // Also verify the layout.
        CPPUNIT_ASSERT_EQUAL( lastname, parseDump("/root/page[1]/body/txt[2]/Special", "rText"));
        CPPUNIT_ASSERT_EQUAL( OUString( "Fixed text." ), parseDump("/root/page[1]/body/txt[1]", ""));
        CPPUNIT_ASSERT_EQUAL( OUString( "" ), parseDump("/root/page[1]/body/txt[4]", ""));
        CPPUNIT_ASSERT_EQUAL( OUString( "Second page." ), parseDump("/root/page[2]/body/txt[1]", ""));
        CPPUNIT_ASSERT_EQUAL( firstname, parseDump("/root/page[2]/body/txt[2]/Special", "rText"));
    }
}

DECLARE_SHELL_MAILMERGE_TEST(testPageBoundariesSimpleMailMerge, "simple-mail-merge.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // This is like the test above, but this one uses the create-single-document-containing-everything-generated approach,
    // and verifies that boundaries of the generated sub-documents are correct inside that document.
    // These boundaries are done using "SwMailMergeOutputPage::documentStartPageNumber<number>" UNO bookmarks (see also
    // SwMailMergeOutputPage::documentStartPageNumber() ).
    executeMailMerge();
    // Here getPages() works on the source document, so get pages of the resulting one.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxMMComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT_EQUAL( sal_uInt16( 19 ), pTextDoc->GetDocShell()->GetWrtShell()->GetPhyPageNum()); // 10 pages, but each sub-document starts on odd page number
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
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxMMComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT_EQUAL( sal_uInt16( 20 ), pTextDoc->GetDocShell()->GetWrtShell()->GetPhyPageNum()); // 20 pages, each sub-document starts on odd page number
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

    uno::Reference<text::XTextDocument> xTextDocument(mxMMComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph(getParagraphOrTable(3, xTextDocument->getText()), uno::UNO_QUERY);
    // Make sure that we assert the right paragraph.
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xParagraph->getString());
    // This paragraph had a bullet numbering, make sure that the list id is not empty.
    CPPUNIT_ASSERT(!getProperty<OUString>(xParagraph, "ListId").isEmpty());
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf90230, "empty.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // MM of an empty document caused an assertion in the SwIndexReg dtor.
    executeMailMerge();
}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
