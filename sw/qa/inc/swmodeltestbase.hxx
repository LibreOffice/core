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
#include <test/unoapixml_test.hxx>
#include <unotools/tempfile.hxx>

#include <doc.hxx>

/**
 * Macro to declare a new test (with full round-trip. To test
 * import only use the CPPUNIT_TEST_FIXTURE macro directly).
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

#define DECLARE_OOXMLEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)
#define DECLARE_RTFEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)
#define DECLARE_ODFEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)
#define DECLARE_FODFEXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)
#define DECLARE_WW8EXPORT_TEST(TestName, filename) DECLARE_SW_ROUNDTRIP_TEST(TestName, filename, nullptr, Test)

#define DECLARE_SW_EXPORT_TEST(TestName, filename, password, BaseClass) \
    class TestName : public BaseClass { \
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

class SwXTextDocument;
namespace vcl
{
namespace pdf
{
class PDFiumDocument;
}
}

/// Base class for filter tests loading or roundtripping a document, then asserting the document model.
class SWQAHELPER_DLLPUBLIC SwModelTestBase : public UnoApiXmlTest
{
private:
    bool mbExported; ///< Does maTempFile already contain something useful?

protected:
    xmlBufferPtr mpXmlBuffer;
    OUString mpFilter;

    sal_uInt32 mnStartTime;

    /// Copy&paste helper.
    void paste(std::u16string_view aFilename, OUString aInstance, css::uno::Reference<css::text::XTextRange> const& xTextRange);

public:
    SwModelTestBase(const OUString& pTestDocumentPath = OUString(), const OUString& pFilter = {});

protected:
    /**
     * Helper func used by each unit test to test the 'export' code.
     * (Loads the requested file, calls 'verify' function, save it to temp file, load the
     * temp file and then calls 'verify' function again)
     */
    void executeLoadVerifyReloadVerify(const char* filename, const char* pPassword = nullptr);

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

    void dumpLayout(const css::uno::Reference< css::lang::XComponent > & rComponent);

    void discardDumpedLayout();

    void calcLayout();

    /// Get the body text of the whole document.
    OUString getBodyText() const;

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

    bool isPropertyVoid(const css::uno::Reference<css::uno::XInterface>& object, const OUString& name) const
    {
        if (!hasProperty(object, name))
            return false;

        css::uno::Reference< css::beans::XPropertySet > properties(object, uno::UNO_QUERY_THROW);
        return !properties->getPropertyValue(name).hasValue();
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

    /// Select shape (counted from 1)
    void selectShape(int number);

    /// Get shape by name
    css::uno::Reference<css::drawing::XShape> getShapeByName(std::u16string_view aName);

    /// Get TextFrame by name
    css::uno::Reference<css::drawing::XShape> getTextFrameByName(const OUString& aName);

    void header();

    void saveAndReload(const OUString& pFilter, const char* pPassword = nullptr);

    /// Combines load() and save().
    void loadAndSave(const char* pName, const char* pPassword = nullptr);

    /// Combines load() and saveAndReload().
    void loadAndReload(const char* pName);

    void finish();

    /// Get page count.
    int getPages() const;

    /// Get shape count.
    int getShapes() const;

    /**
     * Creates a new document to be used with the internal sw/ API.
     *
     * Examples:
     * createSwDoc();
     * createSwDoc("test.fodt");
     * createSwDoc("test.fodt", "test");
     */
    void createSwDoc(const char* pName = nullptr, const char* pPassword = nullptr);

    /**
     * As createSwDoc except a Web Document in Browse Mode
     */
    void createSwWebDoc(const char* pName = nullptr);

    /**
     * As createSwDoc except a Global Document
     */
    void createSwGlobalDoc(const char* pName = nullptr);

    /**
     * Gets SwDoc from loaded component
     */
    SwDoc* getSwDoc();

    /**
     * Gets SwDocShell from loaded component
     */
    SwDocShell* getSwDocShell();

    /**
     * Wraps a reqif-xhtml fragment into an XHTML file, and XML-parses it.
     */
    xmlDocUniquePtr WrapReqifFromTempFile();

    bool isExported(){ return mbExported; }

    void emulateTyping(SwXTextDocument& rTextDoc, const std::u16string_view& rStr);

private:
    void loadURL(OUString const& rURL, const char* pPassword = nullptr);
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
    CPPUNIT_NS::assertEquals( rExpected.Color, rActual.Color, rSourceLine, "different Color" );
    CPPUNIT_NS::assertEquals( rExpected.InnerLineWidth, rActual.InnerLineWidth, rSourceLine, "different InnerLineWidth" );
    CPPUNIT_NS::assertEquals( rExpected.OuterLineWidth, rActual.OuterLineWidth, rSourceLine, "different OuterLineWidth" );
    CPPUNIT_NS::assertEquals( rExpected.LineDistance, rActual.LineDistance, rSourceLine, "different LineDistance" );
    CPPUNIT_NS::assertEquals( rExpected.LineStyle, rActual.LineStyle, rSourceLine, "different LineStyle" );
    CPPUNIT_NS::assertEquals( rExpected.LineWidth, rActual.LineWidth, rSourceLine, "different LineWidth" );
}

#define CPPUNIT_ASSERT_BORDER_EQUAL(aExpected, aActual) \
        assertBorderEqual( aExpected, aActual, CPPUNIT_SOURCELINE() ) \

#endif // INCLUDED_SW_QA_INC_SWMODELTESTBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
