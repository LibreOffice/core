/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_UNOAPI_TEST_HXX
#define INCLUDED_TEST_UNOAPI_TEST_HXX

#include <sal/config.h>

#include <string_view>

#include <rtl/ref.hxx>
#include <test/bootstrapfixture.hxx>
#include <test/testinteractionhandler.hxx>
#include <test/xmltesttools.hxx>
#include <tools/stream.hxx>
#include <unotest/macros_test.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <unotools/tempfile.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

enum ValidationFormat
{
    OOXML,
    ODF,
    MSBINARY,
    PDF
};

enum class TestFilter
{
    NONE,
    CSV,
    DOC,
    DOCBOOK,
    DOCM,
    DOCX,
    DOCX_2007,
    DOTX,
    EPUB,
    FODG,
    FODS,
    FODT,
    HTML_CALC,
    HTML_CALC_WEBQUERY,
    HTML_IMPRESS,
    HTML_WRITER,
    MD,
    MML,
    ODB,
    ODG,
    ODM,
    ODP,
    ODS,
    ODT,
    OTT,
    PDF_WRITER,
    POTX,
    PPT,
    PPTM,
    PPTX,
    PPTX_2007,
    RTF,
    SVG_DRAW,
    SVG_IMPRESS,
    SVG_WRITER,
    TEXT,
    TEXT_ENCODED,
    XHTML_CALC,
    XHTML_DRAW,
    XHTML_WRITER,
    XLS,
    XLSM,
    XLST,
    XLSX,
    XLSX_2007,
};

const std::unordered_map<TestFilter, OUString> TestFilterNames{
    { TestFilter::CSV, u"Text - txt - csv (StarCalc)"_ustr },
    { TestFilter::DOC, u"MS Word 97"_ustr },
    { TestFilter::DOCBOOK, u"DocBook File"_ustr },
    { TestFilter::DOCM, u"Office Open XML Text VBA"_ustr },
    { TestFilter::DOCX, u"Office Open XML Text"_ustr },
    { TestFilter::DOCX_2007, u"MS Word 2007 XML"_ustr },
    { TestFilter::DOTX, u"MS Word 2007 XML Template"_ustr },
    { TestFilter::EPUB, u"EPUB"_ustr },
    { TestFilter::FODG, u"OpenDocument Drawing Flat XML"_ustr },
    { TestFilter::FODS, u"OpenDocument Spreadsheet Flat XML"_ustr },
    { TestFilter::FODT, u"OpenDocument Text Flat XML"_ustr },
    { TestFilter::HTML_CALC, u"HTML (StarCalc)"_ustr },
    { TestFilter::HTML_CALC_WEBQUERY, u"calc_HTML_WebQuery"_ustr },
    { TestFilter::HTML_IMPRESS, u"impress_html_Export"_ustr },
    { TestFilter::HTML_WRITER, u"HTML (StarWriter)"_ustr },
    { TestFilter::MD, u"Markdown"_ustr },
    { TestFilter::MML, u"MathML XML (Math)"_ustr },
    { TestFilter::ODB, u"StarOffice XML (Base)"_ustr },
    { TestFilter::ODG, u"draw8"_ustr },
    { TestFilter::ODM, u"writerglobal8_writer"_ustr },
    { TestFilter::ODP, u"impress8"_ustr },
    { TestFilter::ODS, u"calc8"_ustr },
    { TestFilter::ODT, u"writer8"_ustr },
    { TestFilter::OTT, u"writer8_template"_ustr },
    { TestFilter::PDF_WRITER, u"writer_pdf_Export"_ustr },
    { TestFilter::POTX, u"Impress Office Open XML Template"_ustr },
    { TestFilter::PPT, u"MS PowerPoint 97"_ustr },
    { TestFilter::PPTM, u"Impress MS PowerPoint 2007 XML VBA"_ustr },
    { TestFilter::PPTX, u"Impress Office Open XML"_ustr },
    { TestFilter::PPTX_2007, u"Impress MS PowerPoint 2007 XML"_ustr },
    { TestFilter::RTF, u"Rich Text Format"_ustr },
    { TestFilter::SVG_DRAW, u"draw_svg_Export"_ustr },
    { TestFilter::SVG_IMPRESS, u"impress_svg_Export"_ustr },
    { TestFilter::SVG_WRITER, u"writer_svg_Export"_ustr },
    { TestFilter::TEXT, u"Text"_ustr },
    { TestFilter::TEXT_ENCODED, u"Text (encoded)"_ustr },
    { TestFilter::XHTML_CALC, u"XHTML Calc File"_ustr },
    { TestFilter::XHTML_DRAW, u"XHTML Draw File"_ustr },
    { TestFilter::XHTML_WRITER, u"XHTML Writer File"_ustr },
    { TestFilter::XLS, u"MS Excel 97"_ustr },
    { TestFilter::XLSM, u"Calc Office Open XML VBA"_ustr },
    { TestFilter::XLST, u"Calc MS Excel 2007 XML Template"_ustr },
    { TestFilter::XLSX, u"Calc Office Open XML"_ustr },
    { TestFilter::XLSX_2007, u"Calc MS Excel 2007 XML"_ustr },
};

// basic uno api test class

class OOO_DLLPUBLIC_TEST UnoApiTest : public test::BootstrapFixture,
                                      public unotest::MacrosTest,
                                      public XmlTestTools
{
public:
    UnoApiTest(OUString path);

    virtual void setUp() override;
    virtual void tearDown() override;

    OUString createFileURL(std::u16string_view aFileBase);
    OUString createFilePath(std::u16string_view aFileBase);
    void loadFromURL(const OUString& rURL,
                     const css::uno::Sequence<css::beans::PropertyValue>& rParams = {},
                     const char* pPassword = nullptr);
    void dispose();
    OUString loadFromFile(std::u16string_view aFileBase,
                          const css::uno::Sequence<css::beans::PropertyValue>& rParams = {},
                          const char* pPassword = nullptr);

    css::uno::Any executeMacro(const OUString& rScriptURL,
                               const css::uno::Sequence<css::uno::Any>& rParams = {});

    void save(TestFilter eFilter, const css::uno::Sequence<css::beans::PropertyValue>& rParams = {},
              const char* pPassword = nullptr);
    void saveAndReload(TestFilter eFilter,
                       const css::uno::Sequence<css::beans::PropertyValue>& rParams = {},
                       const char* pPassword = nullptr);

    std::unique_ptr<vcl::pdf::PDFiumDocument> parsePDFExport(const OString& rPassword = OString());

    void createTempCopy(std::u16string_view fileName);

    xmlDocUniquePtr parseExport(OUString const& rStreamName);

    /**
     * Returns an xml stream of an exported file.
     * To be used when the exporter doesn't create zip archives, but single files
     * (like Flat ODF Export)
     */
    xmlDocUniquePtr parseExportedFile();

    void skipValidation() { mbSkipValidation = true; }

    void setImportFilterName(TestFilter eFilterName) { meImportFilterName = eFilterName; }

protected:
    // reference to document component that we are testing
    css::uno::Reference<css::lang::XComponent> mxComponent;

    // In case the test needs to work with two documents at the same time
    css::uno::Reference<css::lang::XComponent> mxComponent2;

    utl::TempFileNamed maTempFile;

    SvMemoryStream maMemory; // Underlying memory for parsed PDF files.

    rtl::Reference<TestInteractionHandler> xInteractionHandler;

    void validate(TestFilter eFilter);

private:
    bool mbSkipValidation;
    OUString m_aBaseString;

    TestFilter meImportFilterName;
};

inline void assertRectangleEqual(const tools::Rectangle& rExpected, const tools::Rectangle& rActual,
                                 const sal_Int32 nTolerance, const CppUnit::SourceLine& rSourceLine)
{
    CPPUNIT_NS::assertDoubleEquals(rExpected.Top(), rActual.Top(), nTolerance, rSourceLine,
                                   "different Top");
    CPPUNIT_NS::assertDoubleEquals(rExpected.Left(), rActual.Left(), nTolerance, rSourceLine,
                                   "different Left");
    CPPUNIT_NS::assertDoubleEquals(rExpected.GetWidth(), rActual.GetWidth(), nTolerance,
                                   rSourceLine, "different Width");
    CPPUNIT_NS::assertDoubleEquals(rExpected.GetHeight(), rActual.GetHeight(), nTolerance,
                                   rSourceLine, "different Height");
}

#define CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aExpected, aActual, aTolerance)              \
    assertRectangleEqual(aExpected, aActual, aTolerance, CPPUNIT_SOURCELINE())

inline void assertPointEqual(const Point& rExpected, const Point& rActual,
                             const sal_Int32 nTolerance, const CppUnit::SourceLine& rSourceLine)
{
    CPPUNIT_NS::assertDoubleEquals(rExpected.X(), rActual.X(), nTolerance, rSourceLine,
                                   "different X");
    CPPUNIT_NS::assertDoubleEquals(rExpected.Y(), rActual.Y(), nTolerance, rSourceLine,
                                   "different Y");
}

#define CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(aExpected, aActual, aTolerance)                  \
    assertPointEqual(aExpected, aActual, aTolerance, CPPUNIT_SOURCELINE())

#endif // INCLUDED_TEST_UNOAPI_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
