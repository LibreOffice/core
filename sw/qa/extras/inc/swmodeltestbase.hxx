/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/text/MailMergeType.hpp>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <dbmgr.hxx>
#include <unoprnms.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>

#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>

using namespace com::sun::star;

#define DEFAULT_STYLE "Default Style"

#define DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, BaseClass) \
    class TestName : public BaseClass { \
    protected: \
        virtual OUString getTestName() SAL_OVERRIDE { return OUString::createFromAscii(#TestName); } \
    public: \
        CPPUNIT_TEST_SUITE(TestName); \
        CPPUNIT_TEST(MailMerge); \
        CPPUNIT_TEST_SUITE_END(); \
    \
        void MailMerge() { \
            executeMailMergeTest(filename, datasource, tablename); \
        } \
        void verify() SAL_OVERRIDE; \
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

/**
 * Maps database URIs to the registered database names for quick lookups
 */
typedef std::map<OUString, OUString> DBuriMap;
DBuriMap aDBuriMap;

/// Base class for filter tests loading or roundtriping a document, then asserting the document model.
class SwModelTestBase : public test::BootstrapFixture, public unotest::MacrosTest
{
protected:
    virtual OUString getTestName() { return OUString(); }

public:
    SwModelTestBase(const char* pTestDocumentPath = "")
        : mpXmlBuffer(0)
        , mpTestDocumentPath(pTestDocumentPath)
        , nCurOutputType(0)
    {
    }

    ~SwModelTestBase()
    {
    }

    virtual void setUp()
    {
        test::BootstrapFixture::setUp();

        mxDesktop.set( com::sun::star::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())) );
    }

    virtual void tearDown()
    {
        if (mxComponent.is())
            mxComponent->dispose();
        // There is a reference cleanup error (fixed in master), so this
        // explicitly releases the XJob reference to prevent the otherwise
        // resulting "pure virtual" segfault.
        if (mxJob.is())
            mxJob.set( NULL );
        if (mxMMComponent.is())
        {
            if (nCurOutputType == text::MailMergeType::SHELL)
            {
                SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxMMComponent.get());
                pTxtDoc->GetDocShell()->DoClose();
            }
            else
                mxMMComponent->dispose();
        }

        test::BootstrapFixture::tearDown();
    }

protected:

    /**
     * Helper func used by each unit test to test the 'mail merge' code.
     *
     * Registers the data source, loads the original file as reference,
     * initializes the mail merge job and its default argument sequence.
     *
     * The 'verify' method actually has to execute the mail merge by
     * calling executeMailMerge() after modifying the job arguments.
     */
    void executeMailMergeTest(const char* filename, const char* datasource, const char* tablename = 0)
    {
        header();
        load(mpTestDocumentPath, filename);

        const OUString aPrefix( "LOMM_" );
        utl::TempFile maTempDir(NULL, true);
        const String aWorkDir = maTempDir.GetURL();
        const OUString aURI( getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(datasource) );
        OUString aDBName = registerDBsource( aURI, aPrefix, aWorkDir );
        initMailMergeJobAndArgs( filename, tablename, aDBName, aPrefix, aWorkDir );

        verify();
        finish();

        ::utl::removeTree(aWorkDir);
    }

    /**
     * Function overloaded by unit test. See DECLARE_SW_*_TEST macros
     */
    virtual void verify()
    {
        CPPUNIT_FAIL( "verify method must be overridden" );
    }

private:
    void dumpLayout()
    {
        // create the xml writer
        mpXmlBuffer = xmlBufferCreate();
        xmlTextWriterPtr pXmlWriter = xmlNewTextWriterMemory(mpXmlBuffer, 0);
        xmlTextWriterStartDocument(pXmlWriter, NULL, NULL, NULL);

        // create the dump
        SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();
        SwRootFrm* pLayout = pDoc->GetCurrentLayout();
        pLayout->dumpAsXml(pXmlWriter);

        // delete xml writer
        xmlTextWriterEndDocument(pXmlWriter);
        xmlFreeTextWriter(pXmlWriter);
    }

    void calcLayout()
    {
        SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
        SwDoc* pDoc = pTxtDoc->GetDocShell()->GetDoc();
        pDoc->GetCurrentViewShell()->CalcLayout();
    }


protected:
    /// Get the length of the whole document.
    int getLength()
    {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        OUStringBuffer aBuf;
        while (xParaEnum->hasMoreElements())
        {
            uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
            uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
            while (xRangeEnum->hasMoreElements())
            {
                uno::Reference<text::XTextRange> xRange(xRangeEnum->nextElement(), uno::UNO_QUERY);
                aBuf.append(xRange->getString());
            }
        }
        return aBuf.getLength();
    }

    /// Get a family of styles, see com.sun.star.style.StyleFamilies for possible values.
    uno::Reference<container::XNameAccess> getStyles(OUString aFamily)
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(aFamily), uno::UNO_QUERY);
        return xStyleFamily;
    }

    /**
     * Extract a value from the layout dump using an XPath expression and an attribute name.
     *
     * If the attribute is omitted, the text of the node is returned.
     */
    OUString parseDump(OString aXPath, OString aAttribute = OString())
    {
        if (!mpXmlBuffer)
            dumpLayout();

        xmlDocPtr pXmlDoc = xmlParseMemory((const char*)xmlBufferContent(mpXmlBuffer), xmlBufferLength(mpXmlBuffer));;

        xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
        xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(aXPath.getStr()), pXmlXpathCtx);
        xmlNodeSetPtr pXmlNodes = pXmlXpathObj->nodesetval;
        CPPUNIT_ASSERT_EQUAL(1, xmlXPathNodeSetGetLength(pXmlNodes));
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
        OUString aRet;
        if (aAttribute.getLength())
            aRet = OUString::createFromAscii((const char*)xmlGetProp(pXmlNode, BAD_CAST(aAttribute.getStr())));
        else
            aRet = OUString::createFromAscii((const char*)XML_GET_CONTENT(pXmlNode));

        xmlFreeDoc(pXmlDoc);

        return aRet;
    }

    template< typename T >
    T getProperty( uno::Any obj, const OUString& name ) const
    {
        uno::Reference< beans::XPropertySet > properties( obj, uno::UNO_QUERY );
        T data = T();
        properties->getPropertyValue( name ) >>= data;
        return data;
    }

    template< typename T >
    T getProperty( uno::Reference< uno::XInterface > obj, const OUString& name ) const
    {
        uno::Reference< beans::XPropertySet > properties( obj, uno::UNO_QUERY );
        T data = T();
        properties->getPropertyValue( name ) >>= data;
        return data;
    }

    bool hasProperty(const uno::Reference<uno::XInterface>& obj, const OUString& name) const
    {
        uno::Reference<beans::XPropertySet> properties(obj, uno::UNO_QUERY_THROW);
        return properties->getPropertySetInfo()->hasPropertyByName(name);
    }

    /// Get number of paragraphs of the document.
    int getParagraphs()
    {
        uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        int nRet = 0;
        while (xParaEnum->hasMoreElements())
        {
            xParaEnum->nextElement();
            nRet++;
        }
        return nRet;
    }

    uno::Reference<text::XTextContent> getParagraphOrTable(int number, uno::Reference<text::XText> xText = uno::Reference<text::XText>()) const
    {
        uno::Reference<container::XEnumerationAccess> paraEnumAccess;
        if (xText.is())
            paraEnumAccess.set(xText, uno::UNO_QUERY);
        else
        {
            uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
            paraEnumAccess.set(textDocument->getText(), uno::UNO_QUERY);
        }
        uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
        for( int i = 1;
             i < number;
             ++i )
            paraEnum->nextElement();
        uno::Reference< text::XTextContent> const xElem(paraEnum->nextElement(),
                uno::UNO_QUERY_THROW);
        return xElem;
    }

    // Get paragraph (counted from 1), optionally check it contains the given text.
    uno::Reference< text::XTextRange > getParagraph( int number, OUString content = OUString() ) const
    {
        uno::Reference<text::XTextRange> const xParagraph(
                getParagraphOrTable(number), uno::UNO_QUERY_THROW);
        if( !content.isEmpty())
            CPPUNIT_ASSERT_EQUAL( content, xParagraph->getString());
        return xParagraph;
    }

    uno::Reference<text::XTextRange> getParagraphOfText(int number, uno::Reference<text::XText> xText) const
    {
        uno::Reference<text::XTextRange> const xParagraph(getParagraphOrTable(number, xText), uno::UNO_QUERY_THROW);
        return xParagraph;
    }

    /// Get run (counted from 1) of a paragraph, optionally check it contains the given text.
    uno::Reference<text::XTextRange> getRun(uno::Reference<text::XTextRange> xParagraph, int number, OUString content = OUString()) const
    {
        uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
        for (int i = 1; i < number; ++i)
            xRunEnum->nextElement();
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        if( !content.isEmpty())
            CPPUNIT_ASSERT_EQUAL( content, xRun->getString());
        return xRun;
    }

    /// Get math formula string of a run.
    OUString getFormula(uno::Reference<text::XTextRange> xRun) const
    {
        uno::Reference<container::XContentEnumerationAccess> xContentEnumAccess(xRun, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xContentEnum(xContentEnumAccess->createContentEnumeration(""), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xFormula(xContentEnum->nextElement(), uno::UNO_QUERY);
        return getProperty<OUString>(getProperty< uno::Reference<beans::XPropertySet> >(xFormula, "Model"), "Formula");
    }

    /// get cell of a table; table can be retrieved with getParagraphOrTable
    uno::Reference<table::XCell> getCell(
            uno::Reference<uno::XInterface> const& xTableIfc,
            OUString const& rCell, OUString const& rContent = OUString())
    {
        uno::Reference<text::XTextTable> const xTable(xTableIfc,
                uno::UNO_QUERY_THROW);
        uno::Reference<table::XCell> const xCell(
                xTable->getCellByName(rCell), uno::UNO_SET_THROW);
        if (!rContent.isEmpty())
        {
            uno::Reference<text::XText> const xCellText(xCell,
                    uno::UNO_QUERY_THROW);
            CPPUNIT_ASSERT_EQUAL(rContent, xCellText->getString());
        }
        return xCell;
    }

    /// Get shape (counted from 1)
    uno::Reference<drawing::XShape> getShape(int number)
    {
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
        uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(number - 1), uno::UNO_QUERY);
        return xShape;
    }

    void header()
    {
        fprintf(stderr, "File tested,Execution Time (ms)\n");
    }

    void load(const char* pDir, const char* pName, bool bCalcLayout = true)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        fprintf(stderr, "%s %s,", pDir, pName);
        m_nStartTime = osl_getGlobalTimer();
        mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.text.TextDocument");
        if (bCalcLayout)
            calcLayout();
    }

    void reload(OUString aFilter)
    {
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aArgs(1);
        aArgs[0].Name = "FilterName";
        aArgs[0].Value <<= aFilter;
        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        xStorable->storeToURL(aTempFile.GetURL(), aArgs);
        uno::Reference<lang::XComponent> xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mxComponent = loadFromDesktop(aTempFile.GetURL(), "com.sun.star.text.TextDocument");
        if (mpXmlBuffer)
        {
            xmlBufferFree(mpXmlBuffer);
            mpXmlBuffer = 0;
        }
        calcLayout();
    }

    /// Save the loaded document to a tempfile. Can be used to check the resulting docx/odt directly as a ZIP file.
    void save(OUString aFilter, utl::TempFile& rTempFile)
    {
        rTempFile.EnableKillingFile();
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aFilterArgs(1);
        aFilterArgs[0].Name = "FilterName";
        aFilterArgs[0].Value <<= aFilter;
        xStorable->storeToURL(rTempFile.GetURL(), aFilterArgs);
    }

    void finish()
    {
        sal_uInt32 nEndTime = osl_getGlobalTimer();
        fprintf(stderr, "%" SAL_PRIuUINT32"\n", nEndTime - m_nStartTime);
        if (mpXmlBuffer)
        {
            xmlBufferFree(mpXmlBuffer);
            mpXmlBuffer = 0;
        }
    }

    /// Get page count.
    int getPages()
    {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
        uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
        xCursor->jumpToLastPage();
        return xCursor->getPage();
    }

    uno::Reference<lang::XComponent> mxComponent;
    xmlBufferPtr mpXmlBuffer;

    template< typename T >
    struct MethodEntry
    {
        const char* pName;
        void (T::*pMethod)();
    };
    sal_uInt32 m_nStartTime;

    uno::Reference< lang::XComponent > mxMMComponent;
    uno::Reference< com::sun::star::task::XJob > mxJob;
    uno::Sequence< beans::NamedValue > mSeqMailMergeArgs;

    const char* mpTestDocumentPath;

    sal_Int16 nCurOutputType;

    virtual OUString registerDBsource( const OUString &aURI, const OUString &aPrefix, const String &aWorkDir )
    {
        OUString aDBName;
        DBuriMap::const_iterator pos = aDBuriMap.find( aURI );
        if (pos == aDBuriMap.end())
        {
            aDBName = SwNewDBMgr::LoadAndRegisterDataSource( aURI, &aPrefix, &aWorkDir );
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

    virtual void initMailMergeJobAndArgs( const char* filename, const char* tablename, const OUString &aDBName,
                                          const OUString &aPrefix, const String &aWorkDir )
    {
        uno::Reference< task::XJob > xJob( getMultiServiceFactory()->createInstance( "com.sun.star.text.MailMerge" ), uno::UNO_QUERY_THROW );
        mxJob.set( xJob );

        int seq_id = 5;
        if (tablename) seq_id += 2;
        mSeqMailMergeArgs.realloc( seq_id );

#define OUSTRING_FROM_PROP( prop ) \
    OUString::createFromAscii(SW_PROP_NAME_STR( prop ) )

        seq_id = 0;
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUSTRING_FROM_PROP( UNO_NAME_OUTPUT_TYPE ), uno::Any( text::MailMergeType::SHELL ) );
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUSTRING_FROM_PROP( UNO_NAME_DOCUMENT_URL ), uno::Any(
                                        ( OUString(getURLFromSrc(mpTestDocumentPath) + OUString::createFromAscii(filename)) ) ) );
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUSTRING_FROM_PROP( UNO_NAME_DATA_SOURCE_NAME ), uno::Any( aDBName ) );
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUSTRING_FROM_PROP( UNO_NAME_OUTPUT_URL ), uno::Any( OUString( aWorkDir ) ) );
        mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUSTRING_FROM_PROP( UNO_NAME_FILE_NAME_PREFIX ), uno::Any( aPrefix ));
        if (tablename)
        {
            mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUSTRING_FROM_PROP( UNO_NAME_DAD_COMMAND_TYPE ), uno::Any( sdb::CommandType::TABLE ) );
            mSeqMailMergeArgs[ seq_id++ ] = beans::NamedValue( OUSTRING_FROM_PROP( UNO_NAME_DAD_COMMAND ), uno::Any( OUString::createFromAscii(tablename) ) );
        }
    }

    virtual void executeMailMerge()
    {
        uno::Any res = mxJob->execute( mSeqMailMergeArgs );

        OUString aCurOutputURL;
        OUString aCurFileNamePrefix;
        const beans::NamedValue *pArguments = mSeqMailMergeArgs.getConstArray();
        bool bOk = true;
        sal_Int32 nArgs = mSeqMailMergeArgs.getLength();

        for (sal_Int32 i = 0; i < nArgs; ++i) {
            const OUString &rName  = pArguments[i].Name;
            const uno::Any &rValue = pArguments[i].Value;

            // all error checking was already done by the MM job execution
            if (rName.equalsAscii( GetPropName( UNO_NAME_OUTPUT_URL ) ))
                bOk &= rValue >>= aCurOutputURL;
            else if (rName.equalsAscii( GetPropName( UNO_NAME_FILE_NAME_PREFIX ) ))
                bOk &= rValue >>= aCurFileNamePrefix;
            else if (rName.equalsAscii( GetPropName( UNO_NAME_OUTPUT_TYPE ) ))
                bOk &= rValue >>= nCurOutputType;
        }

        CPPUNIT_ASSERT(bOk);

        if (nCurOutputType == text::MailMergeType::SHELL)
        {
            CPPUNIT_ASSERT(res >>= mxMMComponent);
            CPPUNIT_ASSERT(mxMMComponent.is());
        }
        else
        {
            CPPUNIT_ASSERT(res == true);
            mxMMComponent = loadFromDesktop( aCurOutputURL + "/" + aCurFileNamePrefix + "0.odt",
                                             "com.sun.star.text.TextDocument");
            CPPUNIT_ASSERT(mxMMComponent.is());
            calcLayout();
        }
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
