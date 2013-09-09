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
#include <unotest/macros_test.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/tempfile.hxx>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>

#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>

using namespace com::sun::star;

#define DEFAULT_STYLE "Default Style"

/// Base class for filter tests loading or roundtriping a document, then asserting the document model.
class SwModelTestBase : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    SwModelTestBase()
        : mpXmlBuffer(0)
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

        test::BootstrapFixture::tearDown();
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

    /// Get a family of auto styles, see com.sun.star.style.StyleFamilies for possible values.
    uno::Reference<style::XAutoStyleFamily> getAutoStyles(OUString aFamily)
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
        std::cerr << "File tested,Execution Time (ms)" << std::endl;
    }

    void load(const char* pDir, const char* pName, bool bCalcLayout = true)
    {
        if (mxComponent.is())
            mxComponent->dispose();
        // Output name early, so in the case of a hang, the name of the hanging input file is visible.
        std::cerr << pName << ",";
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
        std::cerr << (nEndTime - m_nStartTime) << std::endl;
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
};

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
