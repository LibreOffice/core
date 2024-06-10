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

#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/MailMergeType.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/task/XJob.hpp>

#include <vcl/filter/PDFiumLibrary.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/DirectoryHelper.hxx>

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

class MMTest2 : public SwModelTestBase
{
public:
    MMTest2();

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
        maMMTest2Filename = filename;
        header();

        utl::TempFileNamed aTempDir(nullptr, true);
        aTempDir.EnableKillingFile();
        const OUString aWorkDir = aTempDir.GetURL();
        const OUString aURI( createFileURL(OUString::createFromAscii(datasource)) );
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
        createSwDoc(maMMTest2Filename);

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
    uno::Reference< css::task::XJob > mxJob;
    std::vector< beans::NamedValue > mMMargs;
    OUString msMailMergeDocumentURL;
    OUString msMailMergeOutputURL;
    OUString msMailMergeOutputPrefix;
    sal_Int16 mnCurOutputType;
    rtl::Reference< SwXTextDocument > mxSwTextDocument;
    uno::Reference< sdbc::XRowSet > mxCurResultSet;
    const char* maMMTest2Filename;
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
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, nullptr, MMTest2, 0, nullptr)

// Will generate documents as files, use loadMailMergeDocument().
#define DECLARE_FILE_MAILMERGE_TEST(TestName, filename, datasource, tablename) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, "writer8", MMTest2, 0, nullptr)

MMTest2::MMTest2()
    : SwModelTestBase(u"/sw/qa/extras/mailmerge/data/"_ustr, u"writer8"_ustr)
    , mnCurOutputType(0)
    , maMMTest2Filename(nullptr)
{
}

DECLARE_SHELL_MAILMERGE_TEST(tdf125522_shell, "tdf125522.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // prepare unit test and run
    executeMailMerge();

    // reset currently opened layout of the original template,
    // and create the layout of the document with 10 mails inside
    dumpMMLayout();

    // there should be no any text frame in output
    CPPUNIT_ASSERT(mxSwTextDocument);

    const auto & rNodes = mxSwTextDocument->GetDocShell()->GetDoc()->GetNodes();
    for (SwNodeOffset nodeIndex(0); nodeIndex<rNodes.Count(); nodeIndex++)
    {
        SwNode* aNode = rNodes[nodeIndex];
        if (aNode->StartOfSectionNode())
        {
            CPPUNIT_ASSERT(!aNode->StartOfSectionNode()->GetFlyFormat());
        }
    }
}

DECLARE_SHELL_MAILMERGE_TEST(testTd78611_shell, "tdf78611.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // prepare unit test and run
    executeMailMerge();

    // reset currently opened layout of the original template,
    // and create the layout of the document with 10 mails inside
    dumpMMLayout();

    // check: each page (one page is one sub doc) has different paragraphs and header paragraphs.
    // All header paragraphs should have numbering.

    // check first page
    CPPUNIT_ASSERT_EQUAL( u"1"_ustr, parseDump("/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL( u"1.1"_ustr, parseDump("/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL( u"1.2"_ustr, parseDump("/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));

    // check some other pages
    CPPUNIT_ASSERT_EQUAL( u"1"_ustr, parseDump("/root/page[3]/body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL( u"1.1"_ustr, parseDump("/root/page[5]/body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    CPPUNIT_ASSERT_EQUAL( u"1.2"_ustr, parseDump("/root/page[7]/body/txt[10]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
}


DECLARE_FILE_MAILMERGE_TEST(testTd78611_file, "tdf78611.odt", "10-testing-addresses.ods", "testing-addresses")
{
    executeMailMerge(true);
    for (int doc = 0; doc < 10; ++doc)
    {
        loadMailMergeDocument( doc );
        CPPUNIT_ASSERT_EQUAL( u"1"_ustr, parseDump("/root/page[1]/body/txt[6]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
        CPPUNIT_ASSERT_EQUAL( u"1.1"_ustr, parseDump("/root/page[1]/body/txt[8]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
        CPPUNIT_ASSERT_EQUAL( u"1.2"_ustr, parseDump("/root/page[1]/body/txt[10]/SwParaPortion/SwLineLayout/SwFieldPortion"_ostr, "expand"_ostr));
    }
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf122156_shell, "linked-with-condition.odt", "5-with-blanks.ods",
                             "names")
{
    // A document with a linked section hidden on an "empty field" condition
    // For combined documents, hidden sections are removed completely
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    // 5 documents 1 page each, starting at odd page numbers => 9
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(9), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum());
    uno::Reference<container::XIndexAccess> xSections(mxSwTextDocument->getTextSections(),
                                                      uno::UNO_QUERY_THROW);
    // 2 out of 5 dataset records have empty "Title" field => no sections in respective documents
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xSections->getCount());
}

DECLARE_FILE_MAILMERGE_TEST(testTdf122156_file, "linked-with-condition.odt", "5-with-blanks.ods",
                            "names")
{
    // A document with a linked section hidden on an "empty field" condition
    // For separate documents, the sections are removed
    executeMailMerge();
    {
        loadMailMergeDocument(0);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    }
    {
        loadMailMergeDocument(1);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
        uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY_THROW);
        // Record 2 has non-empty "Title" field => section is shown
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect, u"IsVisible"_ustr));
    }
    {
        loadMailMergeDocument(2);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
        uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY_THROW);
        // Record 3 has non-empty "Title" field => section is shown
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect, u"IsVisible"_ustr));
    }
    {
        loadMailMergeDocument(3);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
    }
    {
        loadMailMergeDocument(4);
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent,
                                                                      uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
        uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY_THROW);
        // Record 5 has non-empty "Title" field => section is shown
        CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect, u"IsVisible"_ustr));
    }
}

DECLARE_SHELL_MAILMERGE_TEST(exportDirectToPDF_shell, "linked-with-condition.odt", "5-with-blanks.ods",
                            "names")
{
    executeMailMerge();

    CPPUNIT_ASSERT(mxSwTextDocument.is());

    uno::Reference<css::frame::XController> xController(mxSwTextDocument->getCurrentController());
    CPPUNIT_ASSERT(xController.is());

    uno::Reference<css::text::XTextViewCursorSupplier> xSupplier(xController, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xSupplier.is());

    uno::Reference<css::text::XPageCursor> xPageCursor(xSupplier->getViewCursor(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPageCursor.is());

    xPageCursor->jumpToFirstPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xPageCursor->getPage());

    uno::Reference<css::frame::XFrame> xFrame(xController->getFrame());
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(xFrame, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDispatchProvider.is());

    util::URL aURL;
    aURL.Complete = ".uno:ExportDirectToPDF";
    {
        uno::Reference<css::util::XURLTransformer> xParser(css::util::URLTransformer::create(
                                                           comphelper::getProcessComponentContext()));
        CPPUNIT_ASSERT(xParser.is());
        xParser->parseStrict(aURL);
    }

    uno::Reference<css::frame::XDispatch> xDispatch = xDispatchProvider->queryDispatch(aURL, OUString(), 0);
    CPPUNIT_ASSERT(xDispatch.is());

    const OUString sExportTo(msMailMergeOutputURL + "/ExportDirectToPDF.pdf");
    uno::Sequence <css::beans::PropertyValue> aArgs {
        comphelper::makePropertyValue(u"SynchronMode"_ustr, true),
        comphelper::makePropertyValue(u"URL"_ustr, sExportTo)
    };

    xDispatch->dispatch(aURL, aArgs);
    CPPUNIT_ASSERT(comphelper::DirectoryHelper::fileExists(sExportTo));

    SvFileStream aPDFFile(sExportTo, StreamMode::READ);
    SvMemoryStream aMemory;
    aMemory.WriteStream(aPDFFile);
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
        return;

    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPDFium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
    CPPUNIT_ASSERT(pPdfDocument);
    CPPUNIT_ASSERT_EQUAL(5, pPdfDocument->getPageCount());

    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(0);
    CPPUNIT_ASSERT(pPdfPage);
    CPPUNIT_ASSERT_EQUAL(4, pPdfPage->getObjectCount());
}

DECLARE_SHELL_MAILMERGE_TEST(testTdf121168, "section_ps.odt", "4_v01.ods", "Tabelle1")
{
    // A document starting with a section on a page with non-default page style with header
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);
    // 4 documents 1 page each, starting at odd page numbers => 7
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(7), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum());

    SwDoc* pDocMM = mxSwTextDocument->GetDocShell()->GetDoc();
    SwNodeOffset nSizeMM = pDocMM->GetNodes().GetEndOfContent().GetIndex()
                        - pDocMM->GetNodes().GetEndOfExtras().GetIndex() - 2;
    CPPUNIT_ASSERT_EQUAL(SwNodeOffset(16), nSizeMM);

    // All even pages should be empty, all sub-documents have one page
    const SwRootFrame* pLayout = pDocMM->getIDocumentLayoutAccess().GetCurrentLayout();
    const SwPageFrame* pPageFrm = static_cast<const SwPageFrame*>(pLayout->Lower());
    while (pPageFrm)
    {
        sal_uInt16 nPageNum = pPageFrm->GetPhyPageNum();
        bool bOdd = (1 == (nPageNum % 2));
        CPPUNIT_ASSERT_EQUAL(!bOdd, pPageFrm->IsEmptyPage());
        CPPUNIT_ASSERT_EQUAL(sal_uInt16(bOdd ? 1 : 2), pPageFrm->GetVirtPageNum());
        if (bOdd)
        {
            const SwPageDesc* pDesc = pPageFrm->GetPageDesc();
            CPPUNIT_ASSERT_EQUAL(OUString("Teststyle" + OUString::number(nPageNum / 2 + 1)),
                                 pDesc->GetName());
        }
        pPageFrm = static_cast<const SwPageFrame*>(pPageFrm->GetNext());
    }
}


DECLARE_FILE_MAILMERGE_TEST(testTdf81782_file, "tdf78611.odt", "10-testing-addresses.ods", "testing-addresses")
{
    executeMailMerge(true);
    for (int doc = 0; doc < 10; ++doc)
    {
        loadMailMergeDocument( doc );

        // get document properties
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<document::XDocumentProperties> xDocumentProperties(xDocumentPropertiesSupplier->getDocumentProperties());

        // check if properties were set
        uno::Sequence<OUString> aKeywords(xDocumentProperties->getKeywords());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aKeywords.getLength());
        CPPUNIT_ASSERT_EQUAL(u"one two"_ustr, aKeywords[0]);

        // check title and subject
        CPPUNIT_ASSERT_EQUAL(u"my title"_ustr, xDocumentProperties->getTitle());
        CPPUNIT_ASSERT_EQUAL(u"my subject"_ustr, xDocumentProperties->getSubject());
    }
}

// problem was: field content was duplicated & truncated
DECLARE_SHELL_MAILMERGE_TEST(testTdf81750_shell, "tdf81750.odt", "10-testing-addresses.ods", "testing-addresses")
{
    // prepare unit test and run
    executeMailMerge();

    // reset currently opened layout of the original template,
    // and create the layout of the document with 10 mails inside
    dumpMMLayout();

    // check several pages page
    OUString aExpected(u"Text: Foo "_ustr);
    CPPUNIT_ASSERT_EQUAL( aExpected, parseDump("/root/page[1]/body/txt[2]"_ostr, ""_ostr));
    CPPUNIT_ASSERT_EQUAL( aExpected, parseDump("/root/page[3]/body/txt[2]"_ostr, ""_ostr));
    CPPUNIT_ASSERT_EQUAL( aExpected, parseDump("/root/page[5]/body/txt[2]"_ostr, ""_ostr));
    CPPUNIT_ASSERT_EQUAL( aExpected, parseDump("/root/page[7]/body/txt[2]"_ostr, ""_ostr));
    CPPUNIT_ASSERT_EQUAL( aExpected, parseDump("/root/page[9]/body/txt[2]"_ostr, ""_ostr));
}


DECLARE_FILE_MAILMERGE_TEST(testTdf123057_file, "pagecounttest.ott", "db_pagecounttest.ods", "Sheet1")
{
    uno::Reference<beans::XPropertySet> xSect0, xSect1;
    executeMailMerge(true);

    for (int doc = 0; doc < 4; ++doc)
    {
        loadMailMergeDocument(doc);

        // get document properties
        uno::Reference<text::XTextSectionsSupplier> xSectionsSupplier(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xSections(xSectionsSupplier->getTextSections(), uno::UNO_QUERY_THROW);

        switch (doc)
        {
        case 0:
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSections->getCount());
            xSect0.set(xSections->getByIndex(0), uno::UNO_QUERY_THROW);
            xSect1.set(xSections->getByIndex(1), uno::UNO_QUERY_THROW);

            // both sections visible, page num is 2
            CPPUNIT_ASSERT_EQUAL(2, getPages());
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect0, u"IsVisible"_ustr));
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect1, u"IsVisible"_ustr));
            break;
        case 1:
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
            xSect0.set(xSections->getByIndex(0), uno::UNO_QUERY_THROW);

            // second section removed, page num is 1
            CPPUNIT_ASSERT_EQUAL(1, getPages());
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect0, u"IsVisible"_ustr));
            break;
        case 2:
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSections->getCount());
            xSect0.set(xSections->getByIndex(0), uno::UNO_QUERY_THROW);

            // first section removed, page num is 1
            CPPUNIT_ASSERT_EQUAL(1, getPages());
            CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xSect0, u"IsVisible"_ustr));
            break;
        case 3:
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSections->getCount());
            // both sections removed, page num is 1
            CPPUNIT_ASSERT_EQUAL(1, getPages());
            break;
        }
    }
}

// The document has a header with page number and total page count on page 2
// (which uses page style "Default Style") but doesn't have a header set
// for the first page (which uses page style "First Page").
// Fields in the header hadn't been replaced properly.
DECLARE_SHELL_MAILMERGE_TEST(testTdf128148, "tdf128148.odt", "4_v01.ods", "Tabelle1")
{
    executeMailMerge();
    CPPUNIT_ASSERT(mxSwTextDocument);

    // 4 documents with 2 pages each => 8 pages in total
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), mxSwTextDocument->GetDocShell()->GetWrtShell()->GetPhyPageNum());

    SwDoc* pDocMM = mxSwTextDocument->GetDocShell()->GetDoc();
    uno::Reference<frame::XModel> xModel = mxSwTextDocument->GetDocShell()->GetBaseModel();
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);

    // All odd pages have no header, all even pages should have header with text "Page 2 of 2"
    const SwRootFrame* pLayout = pDocMM->getIDocumentLayoutAccess().GetCurrentLayout();
    const SwPageFrame* pPageFrm = static_cast<const SwPageFrame*>(pLayout->Lower());
    while (pPageFrm)
    {
        const sal_uInt16 nPageNum = pPageFrm->GetPhyPageNum();
        const bool bIsEvenPage = ((nPageNum % 2) == 0);

        const OUString& sPageStyle = pPageFrm->GetPageDesc()->GetName();
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(sPageStyle), uno::UNO_QUERY);

        bool bHeaderIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;

        // first page for every data record shouldn't have header, second should
        CPPUNIT_ASSERT_EQUAL(bIsEvenPage, bHeaderIsOn);
        if (bIsEvenPage)
        {
            // text in header on even pages with correctly replaced fields is "Page 2 of 2"
            uno::Reference<text::XText> xHeaderText;
            xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;
            const OUString sHeaderText = xHeaderText->getString();
            CPPUNIT_ASSERT_EQUAL(u"Page 2 of 2"_ustr, sHeaderText);
        }

        pPageFrm = static_cast<const SwPageFrame*>(pPageFrm->GetNext());
    }
}

DECLARE_MAILMERGE_TEST(testGrabBag, "grabbagtest.docx", "onecell.xlsx", "Sheet1", "MS Word 2007 XML", MMTest2, 0, nullptr)
{
    executeMailMerge(true);

    loadMailMergeDocument(0, ".docx");

    SwXTextDocument *const pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(1), pTextDoc->GetDocShell()->GetWrtShell()->GetPhyPageNum());

    // check grabbag
    uno::Reference<beans::XPropertySet> const xModel(
        mxComponent, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aInteropGrabBag;
    pTextDoc->getPropertyValue(u"InteropGrabBag"_ustr) >>= aInteropGrabBag;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), aInteropGrabBag.getLength());

    // check table border - comes from table style "Tabellenraster"
    uno::Reference<text::XTextTable> const xTable(getParagraphOrTable(1, pTextDoc->getText()), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> const xTableProps(xTable, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(table::TableBorder(
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 18, 0), true,
                table::BorderLine(util::Color(0), 0, 0, 0), true,
                sal_Int16(191), true),
            getProperty<table::TableBorder>(xTableProps, u"TableBorder"_ustr));

    // check font is Arial - comes from theme (wrong result was "" - nothing)
    uno::Reference<text::XText> const xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> const xParaA1(getParagraphOrTable(1, xCell->getText()), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Arial"_ustr, getProperty<OUString>(xParaA1, u"CharFontName"_ustr));
}

} // end of anonymous namespace
namespace com::sun::star::table {

static std::ostream& operator<<(std::ostream& rStream, table::BorderLine const& rLine)
{
    rStream << "BorderLine(" << rLine.Color << "," << rLine.InnerLineWidth << "," << rLine.OuterLineWidth << "," << rLine.LineDistance << ")";
    return rStream;
}

static std::ostream& operator<<(std::ostream& rStream, table::TableBorder const& rBorder)
{
    rStream << "TableBorder(\n  "
        << rBorder.TopLine << "," << static_cast<bool>(rBorder.IsTopLineValid) << ",\n  "
        << rBorder.BottomLine << "," << static_cast<bool>(rBorder.IsBottomLineValid) << ",\n  "
        << rBorder.LeftLine << "," << static_cast<bool>(rBorder.IsLeftLineValid) << ",\n  "
        << rBorder.RightLine << "," << static_cast<bool>(rBorder.IsRightLineValid) << ",\n  "
        << rBorder.HorizontalLine << "," << static_cast<bool>(rBorder.IsHorizontalLineValid) << ",\n  "
        << rBorder.VerticalLine << "," << static_cast<bool>(rBorder.IsVerticalLineValid) << ",\n  "
        << rBorder.Distance << "," << static_cast<bool>(rBorder.IsDistanceValid) << ")";
    return rStream;
}

}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
