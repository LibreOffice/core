/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_INC_SWMODELTESTBASE_HXX
#define INCLUDED_SW_QA_INC_SWMODELTESTBASE_HXX

#include <memory>
#include <string_view>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/xml/AttributeData.hpp>

#include "swqahelperdllapi.h"
#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <test/testinteractionhandler.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/tempfile.hxx>

#include <doc.hxx>

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
#define DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, password, BaseClass) \
    class TestName : public BaseClass { \
        protected:\
    virtual OUString getTestName() override { return #TestName; } \
        public:\
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Load_Verify_Reload_Verify); \
    CPPUNIT_TEST_SUITE_END(); \
    \
    void Load_Verify_Reload_Verify() {\
        executeLoadVerifyReloadVerify(filename, password);\
    }\
    void verify() override;\
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

#define DECLARE_SW_EXPORTONLY_TEST(TestName, filename, password, BaseClass) \
    class TestName : public BaseClass { \
        protected:\
    virtual OUString getTestName() override { return #TestName; } \
        public:\
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Load_Reload_Verify); \
    CPPUNIT_TEST_SUITE_END(); \
    \
    void Load_Reload_Verify() {\
        executeLoadReloadVerify(filename, password);\
    }\
    void verify() override;\
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

#define DECLARE_OOXMLIMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, nullptr, Test)
#define DECLARE_OOXMLEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)
#define DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(TestName, filename) DECLARE_SW_EXPORTONLY_TEST(TestName, filename, nullptr, Test)
#define DECLARE_RTFEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)
#define DECLARE_ODFIMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, nullptr, Test)
#define DECLARE_ODFEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)
#define DECLARE_ODFEXPORT_EXPORTONLY_TEST(TestName, filename) DECLARE_SW_EXPORTONLY_TEST(TestName, filename, nullptr, Test)
#define DECLARE_FODFEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)
#define DECLARE_WW8EXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)

#define DECLARE_SW_IMPORT_TEST(TestName, filename, password, BaseClass) \
    class TestName : public BaseClass { \
        protected:\
    virtual OUString getTestName() override { return #TestName; } \
        public:\
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import); \
    CPPUNIT_TEST_SUITE_END(); \
    \
    void Import() { \
        executeImportTest(filename, password);\
    }\
    void verify() override;\
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

#define DECLARE_SW_EXPORT_TEST(TestName, filename, password, BaseClass) \
    class TestName : public BaseClass { \
        protected:\
    virtual OUString getTestName() override { return #TestName; } \
        public:\
    CPPUNIT_TEST_SUITE(TestName); \
    CPPUNIT_TEST(Import_Export); \
    CPPUNIT_TEST_SUITE_END(); \
    \
    void Import_Export() {\
        executeImportExport(filename, password);\
    }\
    void verify() override;\
    }; \
    CPPUNIT_TEST_SUITE_REGISTRATION(TestName); \
    void TestName::verify()

/// Base class for filter tests loading or roundtripping a document, then asserting the document model.
class SWQAHELPER_DLLPUBLIC SwModelTestBase : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
private:
    OUString maFilterOptions;
    OUString maImportFilterOptions;
    OUString maImportFilterName;

protected:
    css::uno::Reference< css::lang::XComponent > mxComponent;

    rtl::Reference<TestInteractionHandler> xInteractionHandler;

    xmlBufferPtr mpXmlBuffer;
    const OUString mpTestDocumentPath;
    const char* mpFilter;

    sal_uInt32 mnStartTime;
    utl::TempFile maTempFile;
    bool mbExported; ///< Does maTempFile already contain something useful?

protected:

    virtual OUString getTestName() { return OUString(); }

    /// Copy&paste helper.
    void paste(std::u16string_view aFilename, css::uno::Reference<css::text::XTextRange> const& xTextRange);

public:
    void setFilterOptions(const OUString &rFilterOptions)
    {
        maFilterOptions = rFilterOptions;
    }

    void setImportFilterOptions(const OUString &rFilterOptions)
    {
        maImportFilterOptions = rFilterOptions;
    }

    void setImportFilterName(const OUString &rFilterName)
    {
        maImportFilterName = rFilterName;
    }

    SwModelTestBase(const OUString& pTestDocumentPath = OUString(), const char* pFilter = "");

    void setUp() override;

    void tearDown() override;

protected:
    /**
     * Helper func used by each unit test to test the 'import' code.
     * (Loads the requested file and then calls 'verify' method)
     */
    void executeImportTest(const char* filename, const char* pPassword = nullptr);

    /**
     * Helper func used by each unit test to test the 'export' code.
     * (Loads the requested file, calls 'verify' function, save it to temp file, load the
     * temp file and then calls 'verify' function again)
     */
    void executeLoadVerifyReloadVerify(const char* filename, const char* pPassword = nullptr);

    /**
     * Helper func used by each unit test to test the 'export' code.
     * (Loads the requested file, save it to temp file, load the
     * temp file and then calls 'verify' method)
     */
    void executeLoadReloadVerify(const char* filename, const char* pPassword = nullptr);

    /**
     * Helper func used by each unit test to test the 'export' code.
     * (Loads the requested file for document base (this represents
     * the initial document condition), exports with the desired
     * export filter and then calls 'verify' method)
     */
    void executeImportExport(const char* filename, const char* pPassword);

    /**
     * Function overridden by unit test. See DECLARE_SW_*_TEST macros
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
    virtual std::unique_ptr<Resetter> preTest(const char* /*filename*/)
    {
        return nullptr;
    }

    /// Override this function if some special file-specific setup is needed during export test: after load, but before save.
    virtual void postLoad(const char* /*pFilename*/)
    {
    }

    /**
     * Override this function if calc layout is not needed
     */
    virtual bool mustCalcLayoutOf(const char* /*filename*/)
    {
        return true;
    }

    /**
     * Override this function if validation is wanted
     */
    virtual bool mustValidate(const char* /*filename*/) const
    {
        return false;
    }

protected:
    void dumpLayout(const css::uno::Reference< css::lang::XComponent > & rComponent);

    void discardDumpedLayout();

    void calcLayout();

    /// Get the length of the whole document.
    int getLength() const;

    /// Get a family of styles, see com.sun.star.style.StyleFamilies for possible values.
    css::uno::Reference<css::container::XNameAccess> getStyles(const OUString& aFamily);

    /// Get a family of auto styles, see com.sun.star.style.StyleFamilies for possible values.
    css::uno::Reference<css::style::XAutoStyleFamily> getAutoStyles(const OUString& aFamily);

    /// Similar to parseExport(), but this gives the xmlDocPtr of the layout dump.
    xmlDocUniquePtr parseLayoutDump();

    /**
     * Extract a value from the layout dump using an XPath expression and an attribute name.
     *
     * If the attribute is omitted, the text of the node is returned.
     */
    OUString parseDump(const OString& aXPath, const OString& aAttribute = OString());

    template< typename T >
    T getProperty( const css::uno::Any& obj, const OUString& name ) const
    {
        css::uno::Reference< css::beans::XPropertySet > properties( obj, uno::UNO_QUERY_THROW );
        T data;
        if (!css::uno::fromAny(properties->getPropertyValue(name), &data))
        {
            OString const msg("the property is of unexpected type or void: "
                    + OUStringToOString(name, RTL_TEXTENCODING_UTF8));
            CPPUNIT_FAIL(msg.getStr());
        }
        return data;
    }

    template< typename T >
    T getProperty( const css::uno::Reference< css::uno::XInterface >& obj, const OUString& name ) const
    {
        css::uno::Reference< css::beans::XPropertySet > properties( obj, uno::UNO_QUERY_THROW );
        T data = T();
        if (!(properties->getPropertyValue(name) >>= data))
        {
            OString const msg("the property is of unexpected type or void: "
                    + OUStringToOString(name, RTL_TEXTENCODING_UTF8));
            CPPUNIT_FAIL(msg.getStr());
        }
        return data;
    }

    bool hasProperty(const css::uno::Reference<css::uno::XInterface>& obj, const OUString& name) const;

    css::xml::AttributeData getUserDefineAttribute(const css::uno::Any& obj, const OUString& name, const OUString& rValue) const;

    int getParagraphs( css::uno::Reference<text::XText> const & xText );

    /// Get number of paragraphs of the document.
    int getParagraphs();

    css::uno::Reference<css::text::XTextContent> getParagraphOrTable(int number, css::uno::Reference<css::text::XText> const & xText = css::uno::Reference<css::text::XText>()) const;

    // Get paragraph (counted from 1), optionally check it contains the given text.
    css::uno::Reference< css::text::XTextRange > getParagraph( int number, const OUString& content = OUString() ) const;

    sal_Int16 getNumberingTypeOfParagraph(int nPara);

    css::uno::Reference<css::text::XTextRange> getParagraphOfText(int number, css::uno::Reference<css::text::XText> const & xText, const OUString& content = OUString()) const;

    /// get nth object/fly that is anchored AT paragraph
    css::uno::Reference<css::beans::XPropertySet> getParagraphAnchoredObject(
        int const index, css::uno::Reference<css::text::XTextRange> const & xPara) const;

    /// Get run (counted from 1) of a paragraph, optionally check it contains the given text.
    css::uno::Reference<css::text::XTextRange> getRun(uno::Reference<css::text::XTextRange> const & xParagraph, int number, const OUString& content = OUString()) const;

    /// Get math formula string of a run.
    OUString getFormula(css::uno::Reference<css::text::XTextRange> const & xRun) const;

    /// get cell of a table; table can be retrieved with getParagraphOrTable
    css::uno::Reference<css::table::XCell> getCell(
            css::uno::Reference<css::uno::XInterface> const& xTableIfc,
            OUString const& rCell, OUString const& rContent = OUString());

    /// Get shape (counted from 1)
    css::uno::Reference<css::drawing::XShape> getShape(int number);

    /// Get shape by name
    css::uno::Reference<css::drawing::XShape> getShapeByName(std::u16string_view aName);

    /// Get TextFrame by name
    css::uno::Reference<css::drawing::XShape> getTextFrameByName(const OUString& aName);

    void header();

    void load(std::u16string_view pDir, const char* pName, const char* pPassword = nullptr)
    {
        return loadURL(m_directories.getURLFromSrc(pDir) + OUString::createFromAscii(pName), pName, pPassword);
    }

    void setTestInteractionHandler(const char* pPassword, std::vector<beans::PropertyValue>& rFilterOptions);

    void loadURL(OUString const& rURL, const char* pName, const char* pPassword = nullptr);

    void reload(const char* pFilter, const char* filename, const char* pPassword = nullptr);

    /// Save the loaded document to a tempfile. Can be used to check the resulting docx/odt directly as a ZIP file.
    void save(const OUString& aFilterName, utl::TempFile& rTempFile);

    void finish();

    /// Get page count.
    int getPages() const;

    /// Get shape count.
    int getShapes() const;

    /**
     * Given that some problem doesn't affect the result in the importer, we
     * test the resulting file directly, by opening the zip file, parsing an
     * xml stream, and asserting an XPath expression. This method returns the
     * xml stream, so that you can do the asserting.
     */
    xmlDocUniquePtr parseExport(const OUString& rStreamName = OUString("word/document.xml"));

    /**
     * Returns an xml stream of an exported file.
     * To be used when the exporter doesn't create zip archives, but single files
     * (like Flat ODF Export)
     */
    xmlDocUniquePtr parseExportedFile();

    std::unique_ptr<SvStream> parseExportStream(const OUString& url, const OUString& rStreamName);

    xmlDocUniquePtr parseExportInternal(const OUString& url, const OUString& rStreamName);

    /**
     * Helper method to return nodes represented by rXPath.
     */
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;

    /**
     * Creates a new document to be used with the internal sw/ API.
     *
     * Examples:
     * SwDoc* pDoc = createSwDoc();
     * SwDoc* pDoc = createSwDoc(DATA_DIRECTORY, "test.fodt");
     */
    SwDoc* createSwDoc(
        std::u16string_view rDataDirectory = std::u16string_view(), const char* pName = nullptr);
};

/**
 * Test whether the expected and actual borderline parameters are equal
 * and assert if not.
 *
 * @param[in]   rExpected    expected borderline object
 * @param[in]   rActual      actual borderline object
 * @param[in]   rSourceLine  line from where the assertion is called
 * Note: This method is the implementation of CPPUNIT_ASSERT_BORDER_EQUAL, so
 *       use that macro instead.
**/
inline void assertBorderEqual(
    const css::table::BorderLine2& rExpected, const css::table::BorderLine2& rActual,
    const CppUnit::SourceLine& rSourceLine )
{
    CPPUNIT_NS::assertEquals<css::util::Color>( rExpected.Color, rActual.Color, rSourceLine, "different Color" );
    CPPUNIT_NS::assertEquals<sal_Int16>( rExpected.InnerLineWidth, rActual.InnerLineWidth, rSourceLine, "different InnerLineWidth" );
    CPPUNIT_NS::assertEquals<sal_Int16>( rExpected.OuterLineWidth, rActual.OuterLineWidth, rSourceLine, "different OuterLineWidth" );
    CPPUNIT_NS::assertEquals<sal_Int16>( rExpected.LineDistance, rActual.LineDistance, rSourceLine, "different LineDistance" );
    CPPUNIT_NS::assertEquals<sal_Int16>( rExpected.LineStyle, rActual.LineStyle, rSourceLine, "different LineStyle" );
    CPPUNIT_NS::assertEquals<sal_Int32>( rExpected.LineWidth, rActual.LineWidth, rSourceLine, "different LineWidth" );
}

#define CPPUNIT_ASSERT_BORDER_EQUAL(aExpected, aActual) \
        assertBorderEqual( aExpected, aActual, CPPUNIT_SOURCELINE() ) \

inline std::ostream& operator<<(std::ostream& rStrm, const Color& rColor)
{
    rStrm << "Color: R:" << static_cast<int>(rColor.GetRed())
          << " G:" << static_cast<int>(rColor.GetGreen())
          << " B:" << static_cast<int>(rColor.GetBlue())
          << " A:" << static_cast<int>(rColor.GetTransparency());
    return rStrm;
}

#endif // INCLUDED_SW_QA_INC_SWMODELTESTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
