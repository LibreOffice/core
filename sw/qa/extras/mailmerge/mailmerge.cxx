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

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/MailMergeType.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>

#include <tools/urlobj.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <olmenu.hxx>
#include <cmdid.h>
#include <pagefrm.hxx>

/**
 * Maps database URIs to the registered database names for quick lookups
 */
typedef std::map<OUString, OUString> DBuriMap;
static DBuriMap aDBuriMap;

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
                               bool file, int selection, const char* column )
    {
        maMMtestFilename = filename;
        header();
        preTest(filename);

        utl::TempFile aTempDir(nullptr, true);
        const OUString aWorkDir = aTempDir.GetURL();
        const OUString aURI( m_directories.getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(datasource) );
        const OUString aPrefix = column ? OUString::createFromAscii( column ) : "LOMM_";
        const OUString aDBName = registerDBsource( aURI, aWorkDir );
        initMailMergeJobAndArgs( filename, tablename, aDBName, aPrefix, aWorkDir, file, selection, column != nullptr );

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
        uno::Reference< uno::XInterface > xInstance = getMultiServiceFactory()->createInstance( "com.sun.star.sdb.RowSet" );
        uno::Reference< beans::XPropertySet > xRowSetPropSet( xInstance, uno::UNO_QUERY );
        assert( xRowSetPropSet.is() && "failed to get XPropertySet interface from RowSet" );
        if (xRowSetPropSet.is())
        {
            xRowSetPropSet->setPropertyValue( "DataSourceName",    uno::makeAny( aDBName ) );
            xRowSetPropSet->setPropertyValue( "Command",           uno::makeAny( OUString::createFromAscii(tablename) ) );
            xRowSetPropSet->setPropertyValue( "CommandType",       uno::makeAny( sdb::CommandType::TABLE ) );

            uno::Reference< sdbc::XRowSet > xRowSet( xInstance, uno::UNO_QUERY );
            if (xRowSet.is())
                xRowSet->execute(); // build ResultSet from properties
            xCurResultSet.set( xRowSet, uno::UNO_QUERY );
            assert( xCurResultSet.is() && "failed to build ResultSet" );
        }
        return xCurResultSet;
    }

    void initMailMergeJobAndArgs( const char* filename, const char* tablename, const OUString &aDBName,
                                  const OUString &aPrefix, const OUString &aWorkDir, bool file, int nDataSets,
                                  const bool bPrefixIsColumn )
    {
        uno::Reference< task::XJob > xJob( getMultiServiceFactory()->createInstance( "com.sun.star.text.MailMerge" ), uno::UNO_QUERY_THROW );
        mxJob.set( xJob );

        mMMargs.reserve( 15 );

        mMMargs.emplace_back( UNO_NAME_OUTPUT_TYPE, uno::Any( file ? text::MailMergeType::FILE : text::MailMergeType::SHELL ) );
        mMMargs.emplace_back( UNO_NAME_DOCUMENT_URL, uno::Any(
                                         ( OUString( m_directories.getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(filename)) ) ) );
        mMMargs.emplace_back( UNO_NAME_DATA_SOURCE_NAME, uno::Any( aDBName ) );
        mMMargs.emplace_back( UNO_NAME_OUTPUT_URL, uno::Any( aWorkDir ) );
        if (file)
            mMMargs.emplace_back( UNO_NAME_FILE_NAME_PREFIX, uno::Any( aPrefix ) );

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
        uno::Sequence< beans::NamedValue > aSeqMailMergeArgs = comphelper::containerToSequence( mMMargs );
        uno::Any res = mxJob->execute( aSeqMailMergeArgs );

        const beans::NamedValue *pArguments = aSeqMailMergeArgs.getConstArray();
        bool bOk = true;
        bool bMMFilenameFromColumn = false;
        sal_Int32 nArgs = aSeqMailMergeArgs.getLength();

        for (sal_Int32 i = 0; i < nArgs; ++i) {
            const OUString &rName  = pArguments[i].Name;
            const uno::Any &rValue = pArguments[i].Value;

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
        load(mpTestDocumentPath, maMMtestFilename);

        if (mnCurOutputType == text::MailMergeType::SHELL)
        {
            CPPUNIT_ASSERT(res >>= mxMMComponent);
            CPPUNIT_ASSERT(mxMMComponent.is());
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(uno::makeAny(true), res);
            if( !bMMFilenameFromColumn && !bDontLoadResult )
                loadMailMergeDocument( 0 );
        }
    }

    /**
     * Like parseExport(), but for given mail merge document.
     */
    xmlDocPtr parseMailMergeExport(const OUString& rStreamName)
    {
        if (mnCurOutputType != text::MailMergeType::FILE)
            return nullptr;

        OUString name = msMailMergeOutputPrefix + OUString::number( 0 ) + ".odt";
        return parseExportInternal( msMailMergeOutputURL + "/" + name, rStreamName );
    }

    void loadMailMergeDocument( const OUString &filename )
    {
        assert( mnCurOutputType == text::MailMergeType::FILE );
        if (mxComponent.is())
            mxComponent->dispose();
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        std::cout << filename << ",";
        mnStartTime = osl_getGlobalTimer();
        mxComponent = loadFromDesktop(msMailMergeOutputURL + "/" + filename, "com.sun.star.text.TextDocument");
        CPPUNIT_ASSERT( mxComponent.is());
        OString name2 = OUStringToOString( filename, RTL_TEXTENCODING_UTF8 );
        discardDumpedLayout();
        if (mustCalcLayoutOf(name2.getStr()))
            calcLayout();
    }

    /**
     Loads number-th document from mail merge. Requires file output from mail merge.
    */
    void loadMailMergeDocument( int number )
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
        name += OUString::number( number ) + ".odt";
        loadMailMergeDocument( name );
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
    uno::Reference< lang::XComponent > mxMMComponent;
    uno::Reference< sdbc::XRowSet > mxCurResultSet;
    const char* maMMtestFilename;
};

#define DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, file, BaseClass, selection, column) \
    class TestName : public BaseClass { \
    protected: \
        virtual OUString getTestName() override { return OUString(#TestName); } \
    public: \
        CPPUNIT_TEST_SUITE(TestName); \
        CPPUNIT_TEST(MailMerge); \
        CPPUNIT_TEST_SUITE_END(); \
    \
        void MailMerge() { \
            executeMailMergeTest(filename, datasource, tablename, file, selection, column); \
        } \
        void verify() override; \
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

// Will generate the resulting document in mxMMDocument.
#define DECLARE_SHELL_MAILMERGE_TEST(TestName, filename, datasource, tablename) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, false, MMTest, 0, nullptr)

// Will generate documents as files, use loadMailMergeDocument().
#define DECLARE_FILE_MAILMERGE_TEST(TestName, filename, datasource, tablename) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, true, MMTest, 0, nullptr)

#define DECLARE_SHELL_MAILMERGE_TEST_SELECTION(TestName, filename, datasource, tablename, selection) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, false, MMTest, selection, nullptr)

#define DECLARE_FILE_MAILMERGE_TEST_COLUMN(TestName, filename, datasource, tablename, column) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, true, MMTest, 0, column)

int MMTest::documentStartPageNumber( int document ) const
{   // See documentStartPageNumber() .
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
    CPPUNIT_ASSERT_EQUAL(document, pos);
    sal_uInt16 page, dummy;
    shell->Push();
    shell->GotoMark( mark->get());
    shell->GetPageNum( page, dummy );
    shell->Pop(SwCursorShell::PopMode::DeleteCurrent);
    return page;
}

MMTest::MMTest()
    : SwModelTestBase("/sw/qa/extras/mailmerge/data/", "writer8")
    , mnCurOutputType(0)
    , maMMtestFilename(nullptr)
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
    xmlDocPtr pXmlDoc = parseMailMergeExport( "styles.xml" );
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

DECLARE_FILE_MAILMERGE_TEST(testWriterDataSource, "writer-mail-merge.odt", "10-testing-addresses-writer.odt", "testing-addresses-writer")
{
    // This failed as the .odt data source was mapped to the jdbc: protocol.
    executeMailMerge();
    for (int doc = 0; doc < 10; ++doc)
    {
        loadMailMergeDocument(doc);
        CPPUNIT_ASSERT_EQUAL(1, getPages());
        CPPUNIT_ASSERT_EQUAL(OUString("Fixed text."), getRun(getParagraph(1), 1)->getString());
        CPPUNIT_ASSERT_EQUAL(OUString("lastname" + OUString::number(doc + 1)), getRun(getParagraph(2), 1)->getString());
        CPPUNIT_ASSERT_EQUAL(OUString("Another fixed text."), getRun(getParagraph(3), 1)->getString());
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
        CPPUNIT_ASSERT_EQUAL(OUString("Fixed text."), getRun(getParagraph(1), 1)->getString());
        CPPUNIT_ASSERT_EQUAL(OUString("lastname" + OUString::number(doc + 1)), getRun(getParagraph(2), 1)->getString());
        CPPUNIT_ASSERT_EQUAL(OUString("Another fixed text."), getRun(getParagraph(3), 1)->getString());
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
        CPPUNIT_ASSERT_EQUAL( OUString(), getRun( getParagraph( 4 ), 1 )->getString()); // empty para at the end of page 1
        CPPUNIT_ASSERT_EQUAL( OUString( "Second page." ), getRun( getParagraph( 5 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( firstname, getRun( getParagraph( 6 ), 1 )->getString());
        // Also verify the layout.
        CPPUNIT_ASSERT_EQUAL( lastname, parseDump("/root/page[1]/body/txt[2]/Special", "rText"));
        CPPUNIT_ASSERT_EQUAL( OUString( "Fixed text." ), parseDump("/root/page[1]/body/txt[1]", ""));
        CPPUNIT_ASSERT_EQUAL( OUString(), parseDump("/root/page[1]/body/txt[4]", ""));
        CPPUNIT_ASSERT_EQUAL( OUString( "Second page." ), parseDump("/root/page[2]/body/txt[1]", ""));
        CPPUNIT_ASSERT_EQUAL( firstname, parseDump("/root/page[2]/body/txt[2]/Special", "rText"));
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
    CPPUNIT_ASSERT_EQUAL( sal_Int32(IDocumentMarkAccess::GetType( **mark )),
                          sal_Int32(IDocumentMarkAccess::MarkType::TEXT_FIELDMARK ) );
    sal_uLong src_pos = (*mark)->GetMarkPos().nNode.GetIndex();

    // Get the size of the document in nodes
    SwDoc *doc = pTextDoc->GetDocShell()->GetDoc();
    sal_uLong size = doc->GetNodes().GetEndOfContent().GetIndex() - doc->GetNodes().GetEndOfExtras().GetIndex();
    CPPUNIT_ASSERT_EQUAL( sal_uLong(13), size );
    size -= 2; // For common start and end nodes

    // Iterate over all field marks in the target document and check that they
    // are positioned at a multitude of the document size
    SwXTextDocument* pMMTextDoc = dynamic_cast<SwXTextDocument *>(mxMMComponent.get());
    CPPUNIT_ASSERT(pMMTextDoc);
    pIDMA = pMMTextDoc->GetDocShell()->GetDoc()->getIDocumentMarkAccess();
    // The target document has the duplicated amount of bookmarks
    // as the helping uno bookmark from the mail merge is left in the doc
    // TODO should be fixed!
    CPPUNIT_ASSERT_EQUAL(sal_Int32(20), pIDMA->getAllMarksCount());
    std::set<sal_uLong> pages;
    sal_Int32 countFieldMarks = 0;
    for( mark = pIDMA->getAllMarksBegin(); mark != pIDMA->getAllMarksEnd(); ++mark )
    {
        IDocumentMarkAccess::MarkType markType = IDocumentMarkAccess::GetType( **mark );
        if( markType == IDocumentMarkAccess::MarkType::TEXT_FIELDMARK )
        {
            sal_uLong pos = (*mark)->GetMarkPos().nNode.GetIndex() - src_pos;
            CPPUNIT_ASSERT_EQUAL(sal_uLong(0), pos % size);
            CPPUNIT_ASSERT(pages.insert(pos).second);
            countFieldMarks++;
        }
        else // see previous TODO
            CPPUNIT_ASSERT_EQUAL( sal_Int32(markType), sal_Int32(IDocumentMarkAccess::MarkType::UNO_BOOKMARK) );
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(10), countFieldMarks);
}

DECLARE_SHELL_MAILMERGE_TEST_SELECTION(testTdf95292, "linked-labels.odt", "10-testing-addresses.ods", "testing-addresses", 5)
{
    // A document with two labels merged with 5 datasets should result in three pages
    executeMailMerge();

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>( mxComponent.get() );
    CPPUNIT_ASSERT( pTextDoc );
    SwWrtShell *pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    CPPUNIT_ASSERT( pWrtShell->IsLabelDoc() );

    pTextDoc = dynamic_cast<SwXTextDocument *>( mxMMComponent.get() );
    CPPUNIT_ASSERT( pTextDoc );
    pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
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
    sal_uLong nSize = pDoc->GetNodes().GetEndOfContent().GetIndex() - pDoc->GetNodes().GetEndOfExtras().GetIndex();
    nSize -= 2; // The common start and end node
    CPPUNIT_ASSERT_EQUAL( sal_uLong(13), nSize );

    SwXTextDocument* pTextDocMM = dynamic_cast<SwXTextDocument *>(mxMMComponent.get());
    CPPUNIT_ASSERT(pTextDocMM);

    SwDoc *pDocMM = pTextDocMM->GetDocShell()->GetDoc();
    sal_uLong nSizeMM = pDocMM->GetNodes().GetEndOfContent().GetIndex() - pDocMM->GetNodes().GetEndOfExtras().GetIndex();
    nSizeMM -= 2;
    CPPUNIT_ASSERT_EQUAL( sal_uLong(10 * nSize), nSizeMM );

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
        CPPUNIT_ASSERT_EQUAL( OUString( "Fixed text." ), getRun( getParagraph( 1 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString( "lastname" + OUString::number( doc )), getRun( getParagraph( 2 ), 1 )->getString());
        CPPUNIT_ASSERT_EQUAL( OUString( "Another fixed text." ), getRun( getParagraph( 3 ), 1 )->getString());
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

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
