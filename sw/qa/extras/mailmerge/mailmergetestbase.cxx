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

/**
 * Maps database URIs to the registered database names for quick lookups
 */
typedef std::map<OUString, OUString> DBuriMap;
DBuriMap aDBuriMap;

class MailMergeTestBase : public SwModelTestBase
{
public:
    MailMergeTestBase()
        : SwModelTestBase(u"/sw/qa/extras/mailmerge/data/"_ustr, u"writer8"_ustr)
        , mnCurOutputType(0)
        , maMMtestFilename(nullptr)
    {
    }

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
            css::uno::Reference<css::lang::XComponent>(mxCurResultSet, css::uno::UNO_QUERY_THROW)
                ->dispose();
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
    void executeMailMergeTest(const char* filename, const char* datasource, const char* tablename,
                              char const* const filter, int selection, const char* column)
    {
        maMMtestFilename = filename;
        header();

        utl::TempFileNamed aTempDir(nullptr, true);
        aTempDir.EnableKillingFile();
        const OUString aWorkDir = aTempDir.GetURL();
        const OUString aURI(createFileURL(OUString::createFromAscii(datasource)));
        const OUString aPrefix = column ? OUString::createFromAscii(column) : u"LOMM_"_ustr;
        const OUString aDBName = registerDBsource(aURI, aWorkDir);
        initMailMergeJobAndArgs(filename, tablename, aDBName, aPrefix, aWorkDir, filter, selection,
                                column != nullptr);

        verify();
        finish();

        mnCurOutputType = 0;
    }

    OUString registerDBsource(const OUString& aURI, const OUString& aWorkDir)
    {
        OUString aDBName;
        DBuriMap::const_iterator pos = aDBuriMap.find(aURI);
        if (pos == aDBuriMap.end())
        {
            aDBName = SwDBManager::LoadAndRegisterDataSource(aURI, &aWorkDir);
            aDBuriMap.insert(std::pair<OUString, OUString>(aURI, aDBName));
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

    uno::Reference<sdbc::XRowSet> getXResultFromDataset(const char* tablename,
                                                        const OUString& aDBName)
    {
        uno::Reference<sdbc::XRowSet> xCurResultSet;
        uno::Reference<uno::XInterface> xInstance
            = getMultiServiceFactory()->createInstance(u"com.sun.star.sdb.RowSet"_ustr);
        uno::Reference<beans::XPropertySet> xRowSetPropSet(xInstance, uno::UNO_QUERY);
        assert(xRowSetPropSet.is() && "failed to get XPropertySet interface from RowSet");
        if (xRowSetPropSet.is())
        {
            xRowSetPropSet->setPropertyValue(u"DataSourceName"_ustr, uno::Any(aDBName));
            xRowSetPropSet->setPropertyValue(u"Command"_ustr,
                                             uno::Any(OUString::createFromAscii(tablename)));
            xRowSetPropSet->setPropertyValue(u"CommandType"_ustr,
                                             uno::Any(sdb::CommandType::TABLE));

            uno::Reference<sdbc::XRowSet> xRowSet(xInstance, uno::UNO_QUERY);
            if (xRowSet.is())
                xRowSet->execute(); // build ResultSet from properties
            xCurResultSet = xRowSet;
            assert(xCurResultSet.is() && "failed to build ResultSet");
        }
        return xCurResultSet;
    }

    void initMailMergeJobAndArgs(const char* filename, const char* tablename,
                                 const OUString& aDBName, const OUString& aPrefix,
                                 const OUString& aWorkDir, char const* const filter, int nDataSets,
                                 const bool bPrefixIsColumn)
    {
        uno::Reference<task::XJob> xJob(
            getMultiServiceFactory()->createInstance(u"com.sun.star.text.MailMerge"_ustr),
            uno::UNO_QUERY_THROW);
        mxJob.set(xJob);

        mMMargs.reserve(15);

        mMMargs.emplace_back(UNO_NAME_OUTPUT_TYPE, uno::Any(filter ? text::MailMergeType::FILE
                                                                   : text::MailMergeType::SHELL));
        mMMargs.emplace_back(UNO_NAME_DOCUMENT_URL,
                             uno::Any((createFileURL(OUString::createFromAscii(filename)))));
        mMMargs.emplace_back(UNO_NAME_DATA_SOURCE_NAME, uno::Any(aDBName));
        mMMargs.emplace_back(UNO_NAME_OUTPUT_URL, uno::Any(aWorkDir));
        if (filter)
        {
            mMMargs.emplace_back(UNO_NAME_FILE_NAME_PREFIX, uno::Any(aPrefix));
            mMMargs.emplace_back(UNO_NAME_SAVE_FILTER, uno::Any(OUString::createFromAscii(filter)));
        }

        if (bPrefixIsColumn)
            mMMargs.emplace_back(UNO_NAME_FILE_NAME_FROM_COLUMN, uno::Any(true));

        if (tablename)
        {
            mMMargs.emplace_back(UNO_NAME_DAD_COMMAND_TYPE, uno::Any(sdb::CommandType::TABLE));
            mMMargs.emplace_back(UNO_NAME_DAD_COMMAND,
                                 uno::Any(OUString::createFromAscii(tablename)));
        }

        if (nDataSets > 0)
        {
            mxCurResultSet = getXResultFromDataset(tablename, aDBName);
            uno::Reference<sdbcx::XRowLocate> xCurRowLocate(mxCurResultSet, uno::UNO_QUERY);
            mMMargs.emplace_back(UNO_NAME_RESULT_SET, uno::Any(mxCurResultSet));
            std::vector<uno::Any> vResult;
            vResult.reserve(nDataSets);
            sal_Int32 i;
            for (i = 0, mxCurResultSet->first(); i < nDataSets; i++, mxCurResultSet->next())
            {
                vResult.emplace_back(xCurRowLocate->getBookmark());
            }
            mMMargs.emplace_back(UNO_NAME_SELECTION,
                                 uno::Any(comphelper::containerToSequence(vResult)));
        }
    }

    void executeMailMerge(bool bDontLoadResult = false)
    {
        const uno::Sequence<beans::NamedValue> aSeqMailMergeArgs
            = comphelper::containerToSequence(mMMargs);
        uno::Any res = mxJob->execute(aSeqMailMergeArgs);

        bool bOk = true;
        bool bMMFilenameFromColumn = false;

        for (const beans::NamedValue& rArgument : aSeqMailMergeArgs)
        {
            const OUString& rName = rArgument.Name;
            const uno::Any& rValue = rArgument.Value;

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
            uno::Reference<lang::XComponent> xTmp;
            CPPUNIT_ASSERT(res >>= xTmp);
            mxSwTextDocument = dynamic_cast<SwXTextDocument*>(xTmp.get());
            CPPUNIT_ASSERT(mxSwTextDocument.is());
        }
        else
        {
            CPPUNIT_ASSERT_EQUAL(uno::Any(true), res);
            if (!bMMFilenameFromColumn && !bDontLoadResult)
                loadMailMergeDocument(0);
        }
    }

    /**
     * Like parseExport(), but for given mail merge document.
     */
    xmlDocUniquePtr parseMailMergeExport(const OUString& rStreamName)
    {
        if (mnCurOutputType != text::MailMergeType::FILE)
            return nullptr;

        OUString name = msMailMergeOutputPrefix + OUString::number(0) + ".odt";
        std::unique_ptr<SvStream> pStream(
            parseExportStream(msMailMergeOutputURL + "/" + name, rStreamName));

        return parseXmlStream(pStream.get());
    }

    void loadMailMergeDocument(const OUString& filename)
    {
        assert(mnCurOutputType == text::MailMergeType::FILE);
        if (mxComponent.is())
            mxComponent->dispose();
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        std::cout << filename << ",";
        mnStartTime = osl_getGlobalTimer();
        mxComponent = loadFromDesktop(msMailMergeOutputURL + "/" + filename,
                                      u"com.sun.star.text.TextDocument"_ustr);
        calcLayout();
    }

    /**
     Loads number-th document from mail merge. Requires file output from mail merge.
    */
    void loadMailMergeDocument(int number, char const* const ext = ".odt")
    {
        OUString name;
        if (!msMailMergeOutputPrefix.isEmpty())
            name = msMailMergeOutputPrefix;
        else
        {
            INetURLObject aURLObj;
            aURLObj.SetSmartProtocol(INetProtocol::File);
            aURLObj.SetSmartURL(msMailMergeDocumentURL);
            name = aURLObj.GetBase();
        }
        name += OUString::number(number)
                + OStringToOUString(std::string_view(ext, strlen(ext)), RTL_TEXTENCODING_ASCII_US);
        loadMailMergeDocument(name);
    }

    // Returns page number of the first page of a MM document inside the large MM document (used in the SHELL case).
    int documentStartPageNumber(int document) const
    { // See documentStartPageNumber() .
        CPPUNIT_ASSERT(mxSwTextDocument);
        SwWrtShell* shell = mxSwTextDocument->GetDocShell()->GetWrtShell();
        IDocumentMarkAccess* marks = shell->GetDoc()->getIDocumentMarkAccess();
        // Unfortunately, the pages are marked using UNO bookmarks, which have internals names, so they cannot be referred to by their names.
        // Assume that there are no other UNO bookmarks than the ones used by mail merge, and that they are in the sorted order.
        IDocumentMarkAccess::const_iterator_t mark;
        int pos = 0;
        for (mark = marks->getAllMarksBegin(); mark != marks->getAllMarksEnd() && pos < document;
             ++mark)
        {
            if (IDocumentMarkAccess::GetType(**mark) == IDocumentMarkAccess::MarkType::UNO_BOOKMARK)
                ++pos;
        }
        CPPUNIT_ASSERT_EQUAL(document, pos);
        sal_uInt16 page, dummy;
        shell->Push();
        shell->GotoMark(*mark);
        shell->GetPageNum(page, dummy);
        shell->Pop(SwCursorShell::PopMode::DeleteCurrent);
        return page;
    }

protected:
    uno::Reference<css::task::XJob> mxJob;
    std::vector<beans::NamedValue> mMMargs;
    OUString msMailMergeDocumentURL;
    OUString msMailMergeOutputURL;
    OUString msMailMergeOutputPrefix;
    sal_Int16 mnCurOutputType;
    rtl::Reference<SwXTextDocument> mxSwTextDocument;
    uno::Reference<sdbc::XRowSet> mxCurResultSet;
    const char* maMMtestFilename;
};

#define DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, filter, BaseClass,       \
                               selection, column)                                                  \
    class TestName : public BaseClass                                                              \
    {                                                                                              \
    public:                                                                                        \
        CPPUNIT_TEST_SUITE(TestName);                                                              \
        CPPUNIT_TEST(MailMerge);                                                                   \
        CPPUNIT_TEST_SUITE_END();                                                                  \
                                                                                                   \
        void MailMerge()                                                                           \
        {                                                                                          \
            executeMailMergeTest(filename, datasource, tablename, filter, selection, column);      \
        }                                                                                          \
        void verify() override;                                                                    \
    };                                                                                             \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName);                                                     \
    void TestName::verify()

// Will generate the resulting document in mxMMDocument.
#define DECLARE_SHELL_MAILMERGE_TEST(TestName, filename, datasource, tablename)                    \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, nullptr, MailMergeTestBase,  \
                           0, nullptr)

// Will generate documents as files, use loadMailMergeDocument().
#define DECLARE_FILE_MAILMERGE_TEST(TestName, filename, datasource, tablename)                     \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, "writer8",                   \
                           MailMergeTestBase, 0, nullptr)

#define DECLARE_SHELL_MAILMERGE_TEST_SELECTION(TestName, filename, datasource, tablename,          \
                                               selection)                                          \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, nullptr, MailMergeTestBase,  \
                           selection, nullptr)

#define DECLARE_FILE_MAILMERGE_TEST_COLUMN(TestName, filename, datasource, tablename, column)      \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, "writer8",                   \
                           MailMergeTestBase, 0, column)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
