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
#include <vector>

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/MailMergeType.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/task/XJob.hpp>

#include <tools/urlobj.hxx>
#include <comphelper/sequence.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <pagefrm.hxx>
#include <unoprnms.hxx>
#include <dbmgr.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>

namespace
{
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
        if (mxSwTextDocument.is())
        {
            if (mnCurOutputType == text::MailMergeType::SHELL)
                mxSwTextDocument->GetDocShell()->DoClose();
            else
                mxSwTextDocument->dispose();
        }
        if (mxCurResultSet.is())
        {
            css::uno::Reference<css::lang::XComponent>(
                mxCurResultSet, css::uno::UNO_QUERY_THROW)->dispose();
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
    void executeMailMergeTest( const char* filename, const char* datasource, const char* tablename,
                               char const*const filter, int selection, const char* column )
    {
        maMMtestFilename = filename;
        header();

        utl::TempFileNamed aTempDir(nullptr, true);
        aTempDir.EnableKillingFile();
        const OUString aWorkDir = aTempDir.GetURL();
        const OUString aURI( createFileURL(OUString::createFromAscii(datasource) ) );
        const OUString aPrefix = column ? OUString::createFromAscii( column ) : u"LOMM_"_ustr;
        const OUString aDBName = registerDBsource( aURI, aWorkDir );
        initMailMergeJobAndArgs( filename, tablename, aDBName, aPrefix, aWorkDir, filter, selection, column != nullptr );

        verify();
        finish();

        mnCurOutputType = 0;
    }

    OUString registerDBsource( const OUString &aURI, const OUString &aWorkDir )
    {
        OUString aDBName;
        DBuriMap::const_iterator pos = aDBuriMap.find( aURI );
        if (pos == aDBuriMap.end())
        {
            aDBName = SwDBManager::LoadAndRegisterDataSource( aURI, &aWorkDir );
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

    uno::Reference< sdbc::XRowSet > getXResultFromDataset( const char* tablename, const OUString &aDBName )
    {
        uno::Reference< sdbc::XRowSet > xCurResultSet;
        uno::Reference< uno::XInterface > xInstance = getMultiServiceFactory()->createInstance( u"com.sun.star.sdb.RowSet"_ustr );
        uno::Reference< beans::XPropertySet > xRowSetPropSet( xInstance, uno::UNO_QUERY );
        assert( xRowSetPropSet.is() && "failed to get XPropertySet interface from RowSet" );
        if (xRowSetPropSet.is())
        {
            xRowSetPropSet->setPropertyValue( u"DataSourceName"_ustr,    uno::Any( aDBName ) );
            xRowSetPropSet->setPropertyValue( u"Command"_ustr,           uno::Any( OUString::createFromAscii(tablename) ) );
            xRowSetPropSet->setPropertyValue( u"CommandType"_ustr,       uno::Any( sdb::CommandType::TABLE ) );

            uno::Reference< sdbc::XRowSet > xRowSet( xInstance, uno::UNO_QUERY );
            if (xRowSet.is())
                xRowSet->execute(); // build ResultSet from properties
            xCurResultSet = xRowSet;
            assert( xCurResultSet.is() && "failed to build ResultSet" );
        }
        return xCurResultSet;
    }

    void initMailMergeJobAndArgs( const char* filename, const char* tablename, const OUString &aDBName,
                                  const OUString &aPrefix, const OUString &aWorkDir,
                                  char const*const filter, int nDataSets,
                                  const bool bPrefixIsColumn )
    {
        uno::Reference< task::XJob > xJob( getMultiServiceFactory()->createInstance( u"com.sun.star.text.MailMerge"_ustr ), uno::UNO_QUERY_THROW );
        mxJob.set( xJob );

        mMMargs.reserve( 15 );

        mMMargs.emplace_back( UNO_NAME_OUTPUT_TYPE, uno::Any( filter ? text::MailMergeType::FILE : text::MailMergeType::SHELL ) );
        mMMargs.emplace_back( UNO_NAME_DOCUMENT_URL, uno::Any(
                                         ( createFileURL(OUString::createFromAscii(filename)) ) ) );
        mMMargs.emplace_back( UNO_NAME_DATA_SOURCE_NAME, uno::Any( aDBName ) );
        mMMargs.emplace_back( UNO_NAME_OUTPUT_URL, uno::Any( aWorkDir ) );
        if (filter)
        {
            mMMargs.emplace_back( UNO_NAME_FILE_NAME_PREFIX, uno::Any( aPrefix ) );
            mMMargs.emplace_back(UNO_NAME_SAVE_FILTER, uno::Any(OUString::createFromAscii(filter)));
        }

        if (bPrefixIsColumn)
            mMMargs.emplace_back( UNO_NAME_FILE_NAME_FROM_COLUMN, uno::Any( true ) );

        if (tablename)
        {
            mMMargs.emplace_back( UNO_NAME_DAD_COMMAND_TYPE, uno::Any( sdb::CommandType::TABLE ) );
            mMMargs.emplace_back( UNO_NAME_DAD_COMMAND, uno::Any( OUString::createFromAscii(tablename) ) );
        }

        if (nDataSets > 0)
        {
            mxCurResultSet = getXResultFromDataset( tablename, aDBName );
            uno::Reference< sdbcx::XRowLocate > xCurRowLocate( mxCurResultSet, uno::UNO_QUERY );
            mMMargs.emplace_back( UNO_NAME_RESULT_SET, uno::Any( mxCurResultSet ) );
            std::vector< uno::Any > vResult;
            vResult.reserve( nDataSets );
            sal_Int32 i;
            for (i = 0, mxCurResultSet->first(); i < nDataSets; i++, mxCurResultSet->next())
            {
                vResult.emplace_back( xCurRowLocate->getBookmark() );
            }
            mMMargs.emplace_back( UNO_NAME_SELECTION, uno::Any( comphelper::containerToSequence(vResult) ) );
        }

    }

    void executeMailMerge( bool bDontLoadResult = false )
    {
        const uno::Sequence< beans::NamedValue > aSeqMailMergeArgs = comphelper::containerToSequence( mMMargs );
        uno::Any res = mxJob->execute( aSeqMailMergeArgs );

        bool bOk = true;
        bool bMMFilenameFromColumn = false;

        for (const beans::NamedValue& rArgument : aSeqMailMergeArgs) {
            const OUString &rName  = rArgument.Name;
            const uno::Any &rValue = rArgument.Value;

            // all error checking was already done by the MM job execution
            if (rName == UNO_NAME_OUTPUT_URL)
                bOk &= rValue >>= msMailMergeOutputURL;
            else if (rName == UNO_NAME_FILE_NAME_PREFIX)
                bOk &= rValue >>= msMailMergeOutputPrefix;
            else if (rName == UNO_NAME_OUTPUT_TYPE)
                bOk &= rValue >>= mnCurOutputType;
            else if (rName == UNO_NAME_FILE_NAME_FROM_COLUMN)
                bOk &= rValue >>= bMMFilenameFromColumn;
            else if (rName == UNO_NAME_DOCUMENT_URL)
                bOk &= rValue >>= msMailMergeDocumentURL;
        }

        CPPUNIT_ASSERT(bOk);

        // MM via UNO just works with file names. If we load the file on
        // Windows before MM uses it, MM won't work, as it's already open.
        // Don't move the load before the mail merge execution!
        // (see gb_CppunitTest_use_instdir_configuration)
        createSwDoc(maMMtestFilename);

        if (mnCurOutputType == text::MailMergeType::SHELL)
        {
            uno::Reference< lang::XComponent > xTmp;
            CPPUNIT_ASSERT(res >>= xTmp);
            mxSwTextDocument = dynamic_cast<SwXTextDocument*>(xTmp.get());
            CPPUNIT_ASSERT(mxSwTextDocument.is());
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(uno::Any(true), res);
            if( !bMMFilenameFromColumn && !bDontLoadResult )
                loadMailMergeDocument( 0 );
        }
    }

    /**
     * Like parseExport(), but for given mail merge document.
     */
    xmlDocUniquePtr parseMailMergeExport(const OUString& rStreamName)
    {
        if (mnCurOutputType != text::MailMergeType::FILE)
            return nullptr;

        OUString name = msMailMergeOutputPrefix + OUString::number( 0 ) + ".odt";
        std::unique_ptr<SvStream> pStream(parseExportStream(msMailMergeOutputURL + "/" + name, rStreamName));

        return parseXmlStream(pStream.get());
    }

    void loadMailMergeDocument( const OUString &filename )
    {
        assert( mnCurOutputType == text::MailMergeType::FILE );
        if (mxComponent.is())
            mxComponent->dispose();
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        std::cout << filename << ",";
        mnStartTime = osl_getGlobalTimer();
        mxComponent = loadFromDesktop(msMailMergeOutputURL + "/" + filename, u"com.sun.star.text.TextDocument"_ustr);
        discardDumpedLayout();
        calcLayout();
    }

    /**
     Loads number-th document from mail merge. Requires file output from mail merge.
    */
    void loadMailMergeDocument(int number, char const*const ext = ".odt")
    {
        OUString name;
        if (!msMailMergeOutputPrefix.isEmpty())
            name = msMailMergeOutputPrefix;
        else
        {
            INetURLObject aURLObj;
            aURLObj.SetSmartProtocol( INetProtocol::File );
            aURLObj.SetSmartURL( msMailMergeDocumentURL );
            name = aURLObj.GetBase();
        }
        name += OUString::number(number) + OStringToOUString(std::string_view(ext, strlen(ext)), RTL_TEXTENCODING_ASCII_US);
        loadMailMergeDocument( name );
    }

    /**
     Resets currently opened layout of the original template,
     and creates the layout of the document with N mails inside
     (result run with text::MailMergeType::SHELL)
    */
    void dumpMMLayout()
    {
        mpXmlBuffer = xmlBufferPtr();
        dumpLayout(static_cast<SfxBaseModel*>(mxSwTextDocument.get()));
    }

protected:
    // Returns page number of the first page of a MM document inside the large MM document (used in the SHELL case).
    int documentStartPageNumber( int document ) const;

    uno::Reference< css::task::XJob > mxJob;
    std::vector< beans::NamedValue > mMMargs;
    OUString msMailMergeDocumentURL;
    OUString msMailMergeOutputURL;
    OUString msMailMergeOutputPrefix;
    sal_Int16 mnCurOutputType;
    rtl::Reference< SwXTextDocument > mxSwTextDocument;
    uno::Reference< sdbc::XRowSet > mxCurResultSet;
    const char* maMMtestFilename;
};

#define DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, filter, BaseClass, selection, column) \
    class TestName : public BaseClass { \
    public: \
        CPPUNIT_TEST_SUITE(TestName); \
        CPPUNIT_TEST(MailMerge); \
        CPPUNIT_TEST_SUITE_END(); \
    \
        void MailMerge() { \
            executeMailMergeTest(filename, datasource, tablename, filter, selection, column); \
        } \
        void verify() override; \
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

// Will generate the resulting document in mxMMDocument.
#define DECLARE_SHELL_MAILMERGE_TEST(TestName, filename, datasource, tablename) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, nullptr, MMTest, 0, nullptr)

// Will generate documents as files, use loadMailMergeDocument().
#define DECLARE_FILE_MAILMERGE_TEST(TestName, filename, datasource, tablename) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, "writer8", MMTest, 0, nullptr)

#define DECLARE_SHELL_MAILMERGE_TEST_SELECTION(TestName, filename, datasource, tablename, selection) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, nullptr, MMTest, selection, nullptr)

#define DECLARE_FILE_MAILMERGE_TEST_COLUMN(TestName, filename, datasource, tablename, column) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, "writer8", MMTest, 0, column)

int MMTest::documentStartPageNumber( int document ) const
{   // See documentStartPageNumber() .
    CPPUNIT_ASSERT(mxSwTextDocument);
    SwWrtShell* shell = mxSwTextDocument->GetDocShell()->GetWrtShell();
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
    CPPUNIT_ASSERT_EQUAL(document, pos);
    sal_uInt16 page, dummy;
    shell->Push();
    shell->GotoMark( *mark );
    shell->GetPageNum( page, dummy );
    shell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    return page;
}

MMTest::MMTest()
    : SwModelTestBase(u"/sw/qa/extras/mailmerge/data/"_ustr, u"writer8"_ustr)
    , mnCurOutputType(0)
    , maMMtestFilename(nullptr)
{
}

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
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
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
    CPPUNIT_ASSERT_EQUAL( u"graphic"_ustr, getXPath(pXmlDoc, "/office:document-styles/office:styles/style:default-style[1]"_ostr, "family"_ostr));
    CPPUNIT_ASSERT_EQUAL( u"#000000"_ustr, getXPath(pXmlDoc, "/office:document-styles/office:styles/style:default-style[1]/style:graphic-properties"_ostr, "stroke-color"_ostr));
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
        CPPUNIT_ASSERT_EQUAL( lastname, parseDump("/root/page[1]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
        CPPUNIT_ASSERT_EQUAL( u"Fixed text."_ustr, parseDump("/root/page[1]/body/txt[1]"_ostr, ""_ostr));
        CPPUNIT_ASSERT_EQUAL( OUString(), parseDump("/root/page[1]/body/txt[4]"_ostr, ""_ostr));
        CPPUNIT_ASSERT_EQUAL( u"Second page."_ustr, parseDump("/root/page[2]/body/txt[1]"_ostr, ""_ostr));
        CPPUNIT_ASSERT_EQUAL( firstname, parseDump("/root/page[2]/body/txt[2]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
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

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    IDocumentMarkAccess const *pIDMA = pTextDoc->GetDocShell()->GetDoc()->getIDocumentMarkAccess();
    // There is just one mark...
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pIDMA->getAllMarksCount());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pIDMA->getBookmarksCount());
    IDocumentMarkAccess::const_iterator_t mark = pIDMA->getAllMarksBegin();
    // and it's a TEXT_FIELDMARK
    CPPUNIT_ASSERT_EQUAL( sal_Int32(IDocumentMarkAccess::MarkType::TEXT_FIELDMARK),
                            sal_Int32(IDocumentMarkAccess::GetType( **mark )) );
    SwNodeOffset src_pos = (*mark)->GetMarkPos().GetNodeIndex();

    // Get the size of the document in nodes
    SwDoc *doc = pTextDoc->GetDocShell()->GetDoc();
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

    dumpMMLayout();
    xmlDocUniquePtr pLayout(
        xmlParseMemory(reinterpret_cast<const char*>(xmlBufferContent(mpXmlBuffer)),
                       xmlBufferLength(mpXmlBuffer)));

    // check that conditions on sections and bookmarks are evaluated the same
    assertXPath(pLayout, "/root/page"_ostr, 7);
    assertXPath(pLayout, "/root/page[1]/body/section"_ostr, 1);
    assertXPath(pLayout, "/root/page[1]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"In den Bergen war es anstrengend."_ustr);
    assertXPath(pLayout, "/root/page[1]/body/txt[5]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"Mein Urlaub war anstrengend . "_ustr);
    assertXPath(pLayout, "/root/page[3]/body/section"_ostr, 1);
    assertXPath(pLayout, "/root/page[3]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"In Barcelona war es schön."_ustr);
    assertXPath(pLayout, "/root/page[3]/body/txt[5]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"Mein Urlaub war schön . "_ustr);
    assertXPath(pLayout, "/root/page[5]/body/section"_ostr, 1);
    assertXPath(pLayout, "/root/page[5]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"In Paris war es erlebnisreich."_ustr);
    assertXPath(pLayout, "/root/page[5]/body/txt[5]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"Mein Urlaub war erlebnisreich . "_ustr);
    assertXPath(pLayout, "/root/page[7]/body/section"_ostr, 3);
    assertXPath(pLayout, "/root/page[7]/body/section[1]/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"In den Bergen war es anstrengend."_ustr);
    assertXPath(pLayout, "/root/page[7]/body/section[2]/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"In Barcelona war es schön."_ustr);
    assertXPath(pLayout, "/root/page[7]/body/section[3]/txt[1]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"In Paris war es erlebnisreich."_ustr);
    assertXPath(pLayout, "/root/page[7]/body/txt[5]/SwParaPortion/SwLineLayout"_ostr, "portion"_ostr, u"Mein Urlaub war anstrengend schön erlebnisreich . "_ustr);
}

DECLARE_SHELL_MAILMERGE_TEST_SELECTION(testTdf95292, "linked-labels.odt", "10-testing-addresses.ods", "testing-addresses", 5)
{
    // A document with two labels merged with 5 datasets should result in three pages
    executeMailMerge();

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>( mxComponent.get() );
    CPPUNIT_ASSERT( pTextDoc );
    SwWrtShell *pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
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

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    // Get the size of the document in nodes
    SwDoc *pDoc = pTextDoc->GetDocShell()->GetDoc();
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


constexpr char const* const EmptyValuesLegacyData[][8]
    = { { "Heading", "Title: ", "First Name: firstname1", "Last Name: lastname1",
          "Title:  First Name: firstname1", "First Name: firstname1 Last Name: lastname1",
          "Title:  First Name: firstname1 Last Name: lastname1", "Trailing text" },
        { "Heading", "Title: title2", "First Name: ", "Last Name: lastname2",
          "Title: title2 First Name: ", "First Name:  Last Name: lastname2",
          "Title: title2 First Name:  Last Name: lastname2", "Trailing text" },
        { "Heading", "Title: title3", "First Name: firstname3", "Last Name: ",
          "Title: title3 First Name: firstname3", "First Name: firstname3 Last Name: ",
          "Title: title3 First Name: firstname3 Last Name: ", "Trailing text" },
        { "Heading", "Title: ", "First Name: ", "Last Name: lastname4",
          "Title:  First Name: ", "First Name:  Last Name: lastname4",
          "Title:  First Name:  Last Name: lastname4", "Trailing text" },
        { "Heading", "Title: title5", "First Name: ", "Last Name: ", "Title: title5 First Name: ",
          "First Name:  Last Name: ", "Title: title5 First Name:  Last Name: ", "Trailing text" } };
constexpr char const* const EmptyValuesNewData[][8]
    = { { "Heading", "First Name: firstname1", "Last Name: lastname1",
          "Title:  First Name: firstname1", "First Name: firstname1 Last Name: lastname1",
          "Title:  First Name: firstname1 Last Name: lastname1", "Trailing text" },
        { "Heading", "Title: title2", "Last Name: lastname2",
          "Title: title2 First Name: ", "First Name:  Last Name: lastname2",
          "Title: title2 First Name:  Last Name: lastname2", "Trailing text" },
        { "Heading", "Title: title3", "First Name: firstname3",
          "Title: title3 First Name: firstname3", "First Name: firstname3 Last Name: ",
          "Title: title3 First Name: firstname3 Last Name: ", "Trailing text" },
        { "Heading", "Last Name: lastname4", "First Name:  Last Name: lastname4",
          "Title:  First Name:  Last Name: lastname4", "Trailing text" },
        { "Heading", "Title: title5", "Title: title5 First Name: ",
          "Title: title5 First Name:  Last Name: ", "Trailing text" } };


// The following four tests (testEmptyValuesLegacyODT, testEmptyValuesNewODT, (testEmptyValuesLegacyFODT, testEmptyValuesNewFODT)
// check that for native documents without "EmptyDbFieldHidesPara" compatibility option, all paragraphs are exported visible,
// while for documents with the option enabled, the paragraphs with all Database fields having empty values are hidden.

DECLARE_FILE_MAILMERGE_TEST(testEmptyValuesLegacyODT, "tdf35798-legacy.odt", "5-with-blanks.ods",
                            "names")
{
    executeMailMerge();
    for (int doc = 0; doc < 5; ++doc)
    {
        loadMailMergeDocument(doc);
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        pDoc->RemoveInvisibleContent();
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        for (int i = 0; i < 8; ++i)
        {
            auto xPara = getParagraph(i+1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                OString("in doc " + OString::number(doc) + " paragraph " + OString::number(i + 1))
                    .getStr(),
                OUString::createFromAscii(EmptyValuesLegacyData[doc][i]), xPara->getString());
        }
        CPPUNIT_ASSERT_EQUAL(8, getParagraphs());
    }
}

DECLARE_FILE_MAILMERGE_TEST(testEmptyValuesNewODT, "tdf35798-new.odt", "5-with-blanks.ods",
                            "names")
{
    executeMailMerge();
    for (int doc = 0; doc < 5; ++doc)
    {
        loadMailMergeDocument(doc);
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        pDoc->RemoveInvisibleContent();
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        int i;
        for (i = 0; i < 8; ++i)
        {
            const char* pExpected = EmptyValuesNewData[doc][i];
            if (!pExpected)
                break;
            auto xPara = getParagraph(i + 1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                OString("in doc " + OString::number(doc) + " paragraph " + OString::number(i + 1))
                    .getStr(),
                OUString::createFromAscii(pExpected), xPara->getString());
        }
        CPPUNIT_ASSERT_EQUAL(i, getParagraphs());
    }
}

DECLARE_FILE_MAILMERGE_TEST(testEmptyValuesLegacyFODT, "tdf35798-legacy.fodt", "5-with-blanks.ods",
                            "names")
{
    executeMailMerge();
    for (int doc = 0; doc < 5; ++doc)
    {
        loadMailMergeDocument(doc);
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        pDoc->RemoveInvisibleContent();
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        for (int i = 0; i < 8; ++i)
        {
            auto xPara = getParagraph(i + 1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                OString("in doc " + OString::number(doc) + " paragraph " + OString::number(i + 1))
                    .getStr(),
                OUString::createFromAscii(EmptyValuesLegacyData[doc][i]), xPara->getString());
        }
        CPPUNIT_ASSERT_EQUAL(8, getParagraphs());
    }
}

DECLARE_FILE_MAILMERGE_TEST(testEmptyValuesNewFODT, "tdf35798-new.fodt", "5-with-blanks.ods",
                            "names")
{
    executeMailMerge();
    for (int doc = 0; doc < 5; ++doc)
    {
        loadMailMergeDocument(doc);
        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
        pDoc->RemoveInvisibleContent();
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        int i;
        for (i = 0; i < 8; ++i)
        {
            const char* pExpected = EmptyValuesNewData[doc][i];
            if (!pExpected)
                break;
            auto xPara = getParagraph(i + 1);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                OString("in doc " + OString::number(doc) + " paragraph " + OString::number(i + 1))
                    .getStr(),
                OUString::createFromAscii(pExpected), xPara->getString());
        }
        CPPUNIT_ASSERT_EQUAL(i, getParagraphs());
    }
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
