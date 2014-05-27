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
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XAutoStylesSupplier.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/BorderLine2.hpp>

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/mediadescriptor.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>

using namespace css;

#define DEFAULT_STYLE "Default Style"
#define EMU_TO_MM100(EMU) (EMU / 360)

/**
 * Macro to declare a new test (with full round-trip. To test
 * import only use the DECLARE_SW_IMPORT_TEST macro instead).
 * In order to add a new test, one only needs to use this macro
 * and then specify the test content, like this:
 *
 * DECLARE_SW_ROUNDTRIP_TEST(MyTest, "myfilename.docx", Test)
 * {
 *      CPPUNIT_ASSERT_EQUAL(blabla);
 * }
 *
 */
#define DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, BaseClass) \
    class TestName : public BaseClass { \
        protected:\
    virtual OUString getTestName() SAL_OVERRIDE { return OUString::createFromAscii(#TestName); } \
        public:\
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import); \
    CPPUNIT_TEST(Import_Export_Import); \
    CPPUNIT_TEST_SUITE_END(); \
    \
    void Import() { \
        executeImportTest(filename);\
    }\
    void Import_Export_Import() {\
        executeImportExportImportTest(filename);\
    }\
    void verify() SAL_OVERRIDE;\
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

#define DECLARE_SW_IMPORT_TEST(TestName, filename, BaseClass) \
    class TestName : public BaseClass { \
        protected:\
    virtual OUString getTestName() SAL_OVERRIDE { return OUString::createFromAscii(#TestName); } \
        public:\
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import); \
    CPPUNIT_TEST_SUITE_END(); \
    \
    void Import() { \
        executeImportTest(filename);\
    }\
    void verify() SAL_OVERRIDE;\
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

#define DECLARE_SW_EXPORT_TEST(TestName, filename, BaseClass) \
    class TestName : public BaseClass { \
        protected:\
    virtual OUString getTestName() SAL_OVERRIDE { return OUString::createFromAscii(#TestName); } \
        public:\
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import_Export); \
    CPPUNIT_TEST_SUITE_END(); \
    \
    void Import_Export() {\
        executeImportExport(filename);\
    }\
    void verify() SAL_OVERRIDE;\
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

/// Base class for filter tests loading or roundtriping a document, then asserting the document model.
class SwModelTestBase : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    OUString maFilterOptions;

protected:
    uno::Reference<lang::XComponent> mxComponent;
    xmlBufferPtr mpXmlBuffer;
    const char* mpTestDocumentPath;
    const char* mpFilter;

    template<typename T>
    struct MethodEntry
    {
        const char* pName;
        void (T::*pMethod)();
    };

    sal_uInt32 mnStartTime;
    utl::TempFile maTempFile;
    bool mbExported; ///< Does maTempFile already contain something useful?

protected:
    virtual OUString getTestName() { return OUString(); }

public:
    OUString& getFilterOptions()
    {
        return maFilterOptions;
    }
    void setFilterOptions(const OUString &rFilterOptions)
    {
        maFilterOptions = rFilterOptions;
    }

    SwModelTestBase(const char* pTestDocumentPath = "", const char* pFilter = "")
        : mpXmlBuffer(0)
        , mpTestDocumentPath(pTestDocumentPath)
        , mpFilter(pFilter)
        , mnStartTime(0)
        , mbExported(false)
    {
        maTempFile.EnableKillingFile();
    }

    virtual ~SwModelTestBase()
    {}

    virtual void setUp() SAL_OVERRIDE
    {
        test::BootstrapFixture::setUp();

        mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    }

    virtual void tearDown() SAL_OVERRIDE
    {
        if (mxComponent.is())
            mxComponent->dispose();

        test::BootstrapFixture::tearDown();
    }

protected:
    /**
     * Helper func used by each unit test to test the 'import' code.
     * (Loads the requested file and then calls 'verify' method)
     */
    void executeImportTest(const char* filename)
    {
        // If the testcase is stored in some other format, it's pointless to test.
        if (mustTestImportOf(filename))
        {
            header();
            preTest(filename);
            load(mpTestDocumentPath, filename);
            postTest(filename);
            verify();
            finish();
        }
    }

    /**
     * Helper func used by each unit test to test the 'export' code.
     * (Loads the requested file, save it to temp file, load the
     * temp file and then calls 'verify' method)
     */
    void executeImportExportImportTest(const char* filename)
    {
        header();
        preTest(filename);
        load(mpTestDocumentPath, filename);
        reload(mpFilter, filename);
        postTest(filename);
        verify();
        finish();
    }

    /**
     * Helper func used by each unit test to test the 'export' code.
     * (Loads the requested file for document base (this represents
     * the initial document condition), exports with the desired
     * export filter and then calls 'verify' method)
     */
    void executeImportExport(const char* filename)
    {
        header();
        preTest(filename);
        load(mpTestDocumentPath, filename);
        save(OUString::createFromAscii(mpFilter), maTempFile);
        postTest(filename);
        verify();
        finish();
    }

    /**
     * Function overloaded by unit test. See DECLARE_SW_*_TEST macros
     */
    virtual void verify()
    {
        CPPUNIT_FAIL( "verify method must be overridden" );
    }

    /**
     * Override this function if interested in skipping import test for this file
     */
     virtual bool mustTestImportOf(const char* /* filename */) const
     {
        return true;
     }
    /**
     * Override this function if some special filename-specific setup is needed
     */
    virtual void preTest(const char* /*filename*/)
    {
    }

    /**
     * Override this function if some special filename-specific teardown is needed
     */
    virtual void postTest(const char* /*filename*/)
    {
    }

    /**
     * Override this function if not calcing layout is needed
     */
    virtual bool mustCalcLayoutOf(const char* /*filename*/)
    {
        return true;
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
        CPPUNIT_ASSERT(pTxtDoc);
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
    uno::Reference<container::XNameAccess> getStyles(const OUString& aFamily)
    {
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(aFamily), uno::UNO_QUERY);
        return xStyleFamily;
    }

    /// Get a family of auto styles, see com.sun.star.style.StyleFamilies for possible values.
    uno::Reference<style::XAutoStyleFamily> getAutoStyles(const OUString& aFamily)
    {
        uno::Reference< style::XAutoStylesSupplier > xAutoStylesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference< style::XAutoStyles > xAutoStyles(xAutoStylesSupplier->getAutoStyles());
        uno::Reference< style::XAutoStyleFamily > xAutoStyleFamily(xAutoStyles->getByName(aFamily), uno::UNO_QUERY);
        return xAutoStyleFamily;
    }

    /**
     * Extract a value from the layout dump using an XPath expression and an attribute name.
     *
     * If the attribute is omitted, the text of the node is returned.
     */
    OUString parseDump(const OString& aXPath, const OString& aAttribute = OString())
    {
        if (!mpXmlBuffer)
            dumpLayout();

        xmlDocPtr pXmlDoc = xmlParseMemory((const char*)xmlBufferContent(mpXmlBuffer), xmlBufferLength(mpXmlBuffer));;

        xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
        xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(aXPath.getStr()), pXmlXpathCtx);
        xmlNodeSetPtr pXmlNodes = pXmlXpathObj->nodesetval;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("parsing dump failed", 1, xmlXPathNodeSetGetLength(pXmlNodes));
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
    T getProperty( const uno::Any& obj, const OUString& name ) const
    {
        uno::Reference< beans::XPropertySet > properties( obj, uno::UNO_QUERY_THROW );
        T data = T();
        properties->getPropertyValue( name ) >>= data;
        return data;
    }

    template< typename T >
    T getProperty( const uno::Reference< uno::XInterface >& obj, const OUString& name ) const
    {
        uno::Reference< beans::XPropertySet > properties( obj, uno::UNO_QUERY_THROW );
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
    uno::Reference< text::XTextRange > getParagraph( int number, const OUString& content = OUString() ) const
    {
        uno::Reference<text::XTextRange> const xParagraph(
                getParagraphOrTable(number), uno::UNO_QUERY_THROW);
        if( !content.isEmpty())
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "paragraph does not have expected content", content, xParagraph->getString());
        return xParagraph;
    }

    uno::Reference<text::XTextRange> getParagraphOfText(int number, uno::Reference<text::XText> xText, const OUString& content = OUString()) const
    {
        uno::Reference<text::XTextRange> const xParagraph(getParagraphOrTable(number, xText), uno::UNO_QUERY_THROW);
        if (!content.isEmpty())
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "paragraph does not contain expected content", content, xParagraph->getString());
        return xParagraph;
    }

    /// Get run (counted from 1) of a paragraph, optionally check it contains the given text.
    uno::Reference<text::XTextRange> getRun(uno::Reference<text::XTextRange> xParagraph, int number, const OUString& content = OUString()) const
    {
        uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
        for (int i = 1; i < number; ++i)
            xRunEnum->nextElement();
        uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
        if( !content.isEmpty())
            CPPUNIT_ASSERT_EQUAL_MESSAGE( "run does not contain expected content", content, xRun->getString());
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
            CPPUNIT_ASSERT_EQUAL_MESSAGE("cell does not contain expected content", rContent, xCellText->getString());
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

    /// Get TextFrame by name
    uno::Reference<drawing::XShape> getTextFrameByName(const OUString& aName)
    {
        uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xNameAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
        uno::Reference<drawing::XShape> xShape(xNameAccess->getByName(aName), uno::UNO_QUERY);
        return xShape;
    }

    void header()
    {
        std::cout << "File tested,Execution Time (ms)" << std::endl;
    }

    void load(const char* pDir, const char* pName)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        std::cout << pName << ",";
        mnStartTime = osl_getGlobalTimer();
        mxComponent = loadFromDesktop(getURLFromSrc(pDir) + OUString::createFromAscii(pName), "com.sun.star.text.TextDocument");
        if (mustCalcLayoutOf(pName))
            calcLayout();
    }

    void reload(const char* pFilter, const char* filename)
    {
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        OUString aFilterName = OUString::createFromAscii(pFilter);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= aFilterName;
        if (!maFilterOptions.isEmpty())
            aMediaDescriptor["FilterOptions"] <<= maFilterOptions;
        xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
        uno::Reference<lang::XComponent> xComponent(xStorable, uno::UNO_QUERY);
        xComponent->dispose();
        mbExported = true;
        mxComponent = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument");
        if(aFilterName == "Office Open XML Text")
        {
            // too many validation errors right now
            // validate(maTempFile.GetFileName(), test::OOXML);
        }
        else if(aFilterName == "writer8")
        {
            // still a few validation errors
            // validate(maTempFile.GetFileName(), test::ODF);
        }

        if (mpXmlBuffer)
        {
            xmlBufferFree(mpXmlBuffer);
            mpXmlBuffer = 0;
        }
        if (mustCalcLayoutOf(filename))
            calcLayout();
    }

    /// Save the loaded document to a tempfile. Can be used to check the resulting docx/odt directly as a ZIP file.
    void save(const OUString& aFilterName, utl::TempFile& rTempFile)
    {
        rTempFile.EnableKillingFile();
        uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
        utl::MediaDescriptor aMediaDescriptor;
        aMediaDescriptor["FilterName"] <<= aFilterName;
        if (!maFilterOptions.isEmpty())
            aMediaDescriptor["FilterOptions"] <<= maFilterOptions;
        xStorable->storeToURL(rTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    }

    void finish()
    {
        sal_uInt32 nEndTime = osl_getGlobalTimer();
        std::cout << (nEndTime - mnStartTime) << std::endl;
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

    /**
     * Given that some problem doesn't affect the result in the importer, we
     * test the resulting file directly, by opening the zip file, parsing an
     * xml stream, and asserting an XPath expression. This method returns the
     * xml stream, so that you can do the asserting.
     */
    xmlDocPtr parseExport(const OUString& rStreamName = OUString("word/document.xml"))
    {
        if (!mbExported)
            return 0;

        // Read the XML stream we're interested in.
        uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
        uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(rStreamName), uno::UNO_QUERY);
        boost::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));

        return parseXmlStream(pStream.get());
    }

    /**
     * Helper method to return nodes represented by rXPath.
     */
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) SAL_OVERRIDE
    {
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w"), BAD_CAST("http://schemas.openxmlformats.org/wordprocessingml/2006/main"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("v"), BAD_CAST("urn:schemas-microsoft-com:vml"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("mc"), BAD_CAST("http://schemas.openxmlformats.org/markup-compatibility/2006"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("wps"), BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingShape"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("wpg"), BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingGroup"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("wp"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("wp14"), BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("a"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/main"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pic"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/picture"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("rels"), BAD_CAST("http://schemas.openxmlformats.org/package/2006/relationships"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w14"), BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordml"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("m"), BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/math"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("ContentType"), BAD_CAST("http://schemas.openxmlformats.org/package/2006/content-types"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("lc"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/lockedCanvas"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("extended-properties"), BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"));
        xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("a14"), BAD_CAST("http://schemas.microsoft.com/office/drawing/2010/main"));
    }
};

/**
 * Test whether the expected and actual borderline parameters are equal
 * and assert if not.
 *
 * @param[in]   rExpected    expected borderline object
 * @param[in]   rActual      actual borderline object
 * @param[in]   rSourceLine  line from where the assertion is called
 * Note: This method is the implementatition of CPPUNIT_ASSERT_BORDER_EQUAL, so
 *       use that macro instead.
**/
inline void assertBorderEqual(
    const table::BorderLine2& rExpected, const table::BorderLine2& rActual,
    const CppUnit::SourceLine& rSourceLine )
{
    CPPUNIT_NS::assertEquals<util::Color>( rExpected.Color, rActual.Color, rSourceLine, "different Color" );
    CPPUNIT_NS::assertEquals<sal_Int16>( rExpected.InnerLineWidth, rActual.InnerLineWidth, rSourceLine, "different InnerLineWidth" );
    CPPUNIT_NS::assertEquals<sal_Int16>( rExpected.OuterLineWidth, rActual.OuterLineWidth, rSourceLine, "different OuterLineWidth" );
    CPPUNIT_NS::assertEquals<sal_Int16>( rExpected.LineDistance, rActual.LineDistance, rSourceLine, "different LineDistance" );
    CPPUNIT_NS::assertEquals<sal_Int16>( rExpected.LineStyle, rActual.LineStyle, rSourceLine, "different LineStyle" );
    CPPUNIT_NS::assertEquals<sal_Int32>( rExpected.LineWidth, rActual.LineWidth, rSourceLine, "different LineWidth" );
}

#define CPPUNIT_ASSERT_BORDER_EQUAL(aExpected, aActual) \
        assertBorderEqual( aExpected, aActual, CPPUNIT_SOURCELINE() ) \

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
