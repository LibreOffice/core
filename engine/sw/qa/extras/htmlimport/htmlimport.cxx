/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <swmodeltestbase.hxx>

#include <algorithm>
#include <atomic>
#include <string>
#include <utility>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier2.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/embed/XInplaceObject.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/util/XLinkUpdate.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <cpo/uno/Sequence.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <osl/socket.hxx>
#include <osl/thread.hxx>
#include <comphelper/compbase.hxx>
#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Security.hxx>
#include <rtl/strbuf.hxx>
#include <test/commontesttools.hxx>
#include <tools/datetime.hxx>
#include <sfx2/linkmgr.hxx>
#include <comphelper/propertyvalue.hxx>
#include <sot/exchange.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/transfer.hxx>

#include <docsh.hxx>
#include <editsh.hxx>
#include <ndgrf.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>
#include <fmtflcnt.hxx>
#include <fmtfsize.hxx>
#include <frameformats.hxx>
#include <swdtflvr.hxx>
#include <unotxdoc.hxx>
#include <wrtsh.hxx>

using namespace css;
using namespace ::cpo;
using namespace ::cpo::uno;

namespace
{
class LocalImageServer final : public osl::Thread
{
public:
    explicit LocalImageServer(OString aImageData)
        : m_aAddress(u"127.0.0.1"_ustr, 0)
        , m_aImageData(std::move(aImageData))
    {
    }

    ~LocalImageServer() { stop(); }

    bool start()
    {
        m_aAcceptorSocket.setOption(osl_Socket_OptionReuseAddr, 1);
        if (!m_aAcceptorSocket.bind(m_aAddress) || !m_aAcceptorSocket.listen())
        {
            m_aAcceptorSocket.close();
            return false;
        }

        m_nPort = m_aAcceptorSocket.getLocalPort();
        if (m_nPort <= 0)
        {
            m_aAcceptorSocket.close();
            return false;
        }

        if (!create())
        {
            m_aAcceptorSocket.close();
            return false;
        }
        m_bStarted = true;
        return true;
    }

    void stop()
    {
        if (!m_bStarted)
            return;

        m_bStopping.store(true);
        m_aAcceptorSocket.close();
        join();
        m_bStarted = false;
    }

    sal_Int32 getPort() const { return m_nPort; }
    sal_Int32 getSingleRequestCount() const { return m_nSingleRequestCount.load(); }
    sal_Int32 getRepeatedRequestCount() const { return m_nRepeatedRequestCount.load(); }
    sal_Int32 getConnectionCount() const { return m_nConnectionCount.load(); }
    const std::string& getRequestLog() const { return m_aRequestLog; }

private:
    void run() override
    {
        while (!m_bStopping.load())
        {
            osl::StreamSocket aClient;
            const auto nResult = m_aAcceptorSocket.acceptConnection(aClient);
            if (nResult != osl_Socket_Ok)
                break;
            ++m_nConnectionCount;

            std::string aRequest;
            while (aRequest.find("\r\n") == std::string::npos && aRequest.size() < 4096)
            {
                char aBuffer[512];
                const sal_Int32 nRead = aClient.recv(aBuffer, sizeof(aBuffer));
                if (nRead <= 0)
                    break;
                aRequest.append(aBuffer, nRead);
            }
            if (aRequest.empty())
            {
                m_aRequestLog.append("(connection closed without a request)\n");
                aClient.close();
                continue;
            }
            const size_t nLineEnd = aRequest.find("\r\n");
            m_aRequestLog.append(aRequest, 0,
                                 nLineEnd == std::string::npos ? aRequest.size() : nLineEnd);
            m_aRequestLog.push_back('\n');

            const bool bSingleImage = aRequest.find("/single.jpg") != std::string::npos;
            const bool bRepeatedImage = aRequest.find("/repeated.jpg") != std::string::npos;
            const bool bOptions = aRequest.rfind("OPTIONS ", 0) == 0;
            const bool bGet = aRequest.rfind("GET ", 0) == 0;
            if (bSingleImage || bRepeatedImage)
            {
                if (bGet)
                {
                    if (bSingleImage)
                        ++m_nSingleRequestCount;
                    else
                        ++m_nRepeatedRequestCount;
                }

                if (bOptions)
                {
                    static constexpr char sOptions[]
                        = "HTTP/1.1 200 OK\r\nAllow: GET, HEAD, OPTIONS\r\nContent-Length: 0\r\n"
                          "Connection: close\r\n\r\n";
                    writeAll(aClient, sOptions, sizeof(sOptions) - 1);
                }
                else
                {
                    const OString aHeaderString = OString::Concat("HTTP/1.1 200 OK\r\n"
                                                                  "Content-Type: image/jpeg\r\n"
                                                                  "Cache-Control: no-store\r\n"
                                                                  "Content-Length: ")
                                                  + OString::number(m_aImageData.getLength())
                                                  + "\r\nConnection: close\r\n\r\n";
                    writeAll(aClient, aHeaderString.getStr(), aHeaderString.getLength());
                    if (bGet)
                        writeAll(aClient, m_aImageData.getStr(), m_aImageData.getLength());
                }
            }
            else
            {
                static constexpr char sNotFound[]
                    = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
                writeAll(aClient, sNotFound, sizeof(sNotFound) - 1);
            }

            aClient.close();
        }
    }

    static void writeAll(osl::StreamSocket& rSocket, const char* pData, sal_Int32 nLength)
    {
        while (nLength > 0)
        {
            const sal_Int32 nWritten = rSocket.write(pData, nLength);
            if (nWritten <= 0)
                return;
            pData += nWritten;
            nLength -= nWritten;
        }
    }

    osl::SocketAddr m_aAddress;
    osl::AcceptorSocket m_aAcceptorSocket;
    OString m_aImageData;
    sal_Int32 m_nPort = 0;
    std::atomic<bool> m_bStopping = false;
    std::atomic<sal_Int32> m_nSingleRequestCount = 0;
    std::atomic<sal_Int32> m_nRepeatedRequestCount = 0;
    std::atomic<sal_Int32> m_nConnectionCount = 0;
    std::string m_aRequestLog;
    bool m_bStarted = false;
};

class ScopedWebDAVHTTPProvider final
{
public:
    ScopedWebDAVHTTPProvider()
    {
        const auto xContext = comphelper::getProcessComponentContext();
        mxBroker = ucb::UniversalContentBroker::create(xContext);
        mxProviderManager = mxBroker;
        CPPUNIT_ASSERT(mxProviderManager.is());

        uno::Reference<cpo::uno::XInterface> xInstance
            = xContext->getServiceManager()->createInstanceWithContext(
                u"com.sun.star.ucb.WebDAVContentProvider"_ustr, xContext);
        mxProvider.set(xInstance, uno::UNO_QUERY);
        CPPUNIT_ASSERT(mxProvider.is());
        mxProviderManager->registerContentProvider(mxProvider, u"http"_ustr, true);
    }

    ~ScopedWebDAVHTTPProvider()
    {
        if (!mxProviderManager.is() || !mxProvider.is())
            return;

        mxProviderManager->deregisterContentProvider(mxProvider, u"http"_ustr);
    }

private:
    uno::Reference<ucb::XUniversalContentBroker> mxBroker;
    uno::Reference<ucb::XContentProviderManager> mxProviderManager;
    uno::Reference<ucb::XContentProvider> mxProvider;
};

class HtmlImportTest : public SwModelTestBase
{
    public:
        HtmlImportTest() : SwModelTestBase(u"sw/qa/extras/htmlimport/data/"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testPictureImport)
{
    createSwWebDoc("picture.html");
    // The document contains two pictures stored as a link.
    SwEditShell* const pEditShell(getSwDoc()->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rLinkManager.GetLinks().size());
    rLinkManager.Remove(0,2);
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    // TODO: Get the data into clipboard in html format and paste

    // But when pasting we don't want images to be linked.
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testInlinedImage)
{
    createSwWebDoc("inlined_image.html");
    // The document contains only one embedded picture inlined in img's src attribute.

    SwDoc* pDoc = getSwDoc();
    SwEditShell* pEditShell = pDoc->GetEditShell();
    CPPUNIT_ASSERT(pEditShell);

    // This was 1 before 3914a711060341345f15b83656457f90095f32d6
    const sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    uno::Reference<drawing::XShape> xShape = getShape(1);
    uno::Reference<container::XNamed> const xNamed(xShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(u"Image1"_ustr, xNamed->getName());

    uno::Reference<graphic::XGraphic> xGraphic = getProperty< uno::Reference<graphic::XGraphic> >(xShape, u"Graphic"_ustr);
    CPPUNIT_ASSERT(xGraphic.is());
    CPPUNIT_ASSERT(xGraphic->getType() != graphic::GraphicType::EMPTY);

    for (SwNodeOffset n(0); ; n++)
    {
        SwNode* pNode = pDoc->GetNodes()[ n ];
        if (SwGrfNode *pGrfNode = pNode->GetGrfNode())
        {
            // FIXME? For some reason without the fix in 72703173066a2db5c977d422ace
            // I was getting GraphicType::NONE from SwEditShell::GetGraphicType() when
            // running LibreOffice but cannot reproduce that in a unit test here. :-(
            // So, this does not really test anything.
            CPPUNIT_ASSERT(pGrfNode->GetGrfObj().GetType() != GraphicType::NONE);
            break;
        }
    }
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testInlinedImagesPageAndParagraph)
{
    createSwWebDoc("PageAndParagraphFilled.html");

    // The document contains embedded pictures inlined for PageBackground and
    // ParagraphBackground, check for their existence after import
    SwEditShell* const pEditShell(getSwDoc()->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);

    // images are not linked, check for zero links
    const sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    // get the pageStyle where the PageBackgroundFill is defined. Caution: for
    // HTML mode this is *not* called 'Default Style', but 'HTML'. Name is empty
    // due to being loaded embedded. BitmapMode is repeat.
    uno::Reference<style::XStyle> xPageProperties1(getStyles(u"PageStyles"_ustr)->getStyleByName(u"HTML"_ustr));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xPageProperties1, u"FillStyle"_ustr));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xPageProperties1, u"FillBitmapName"_ustr));
    CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_REPEAT, getProperty<drawing::BitmapMode>(xPageProperties1, u"FillBitmapMode"_ustr));

    // we should have one paragraph
    const int nParagraphs = getParagraphs();
    CPPUNIT_ASSERT_EQUAL(1, nParagraphs);

    if(nParagraphs)
    {
        // get the paragraph
        uno::Reference<text::XTextRange> xPara = getParagraph(1);
        uno::Reference< beans::XPropertySet > xParagraphProperties( xPara, uno::UNO_QUERY);

        // check for Bitmap FillStyle, name empty, repeat
        CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_BITMAP, getProperty<drawing::FillStyle>(xParagraphProperties, u"FillStyle"_ustr));
        CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xParagraphProperties, u"FillBitmapName"_ustr));
        CPPUNIT_ASSERT_EQUAL(drawing::BitmapMode_REPEAT, getProperty<drawing::BitmapMode>(xParagraphProperties, u"FillBitmapMode"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testListStyleType)
{
    createSwWebDoc("list-style.html");
    // check unnumbered list style - should be type circle here
    uno::Reference< beans::XPropertySet > xParagraphProperties(getParagraph(4),
                                                               uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xParagraphProperties->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
    cpo::uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    bool bBulletFound=false;
    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "BulletChar")
        {
            // should be 'o'.
            CPPUNIT_ASSERT_EQUAL(u"\uE009"_ustr, rProp.Value.get<OUString>());
            bBulletFound = true;
            break;
        }
    }
    CPPUNIT_ASSERT_MESSAGE("no BulletChar property found for para 4", bBulletFound);

    // check numbered list style - should be type lower-alpha here
    xParagraphProperties.set(getParagraph(14),
                             uno::UNO_QUERY);
    xLevels.set(xParagraphProperties->getPropertyValue(u"NumberingRules"_ustr),
                uno::UNO_QUERY);
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (beans::PropertyValue const& rProp : aProps)
    {
        if (rProp.Name == "NumberingType")
        {
            printf("style is %d\n", rProp.Value.get<sal_Int16>());
            // is lower-alpha in input, translates into chars_lower_letter here
            CPPUNIT_ASSERT_EQUAL(style::NumberingType::CHARS_LOWER_LETTER,
                                 rProp.Value.get<sal_Int16>());
            return;
        }
    }
    CPPUNIT_FAIL("no NumberingType property found for para 14");
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testMetaIsoDates)
{
    createSwWebDoc("meta-ISO8601-dates.html");
    SwDocShell* pDocShell(getSwDocShell());
    uno::Reference<document::XDocumentProperties> xDocProps;

    CPPUNIT_ASSERT(pDocShell);
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pDocShell->GetModel(), uno::UNO_QUERY);
    xDocProps.set(xDPS->getDocumentProperties());

    // get the document properties
    CPPUNIT_ASSERT(xDocProps.is());
    DateTime aCreated(xDocProps->getCreationDate()); // in the new format
    DateTime aModified(xDocProps->getModificationDate()); // in the legacy format (what LibreOffice used to write)

    CPPUNIT_ASSERT_EQUAL(DateTime(Date(7, 5, 2017), tools::Time(12, 34, 3, 921000000)), aCreated);
    CPPUNIT_ASSERT_EQUAL(DateTime(Date(8, 5, 2017), tools::Time(12, 47, 0, 386000000)), aModified);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageWidthAuto)
{
    createSwWebDoc("image-width-auto.html");
    SwEditShell* const pEditShell(getSwDoc()->GetEditShell());
    CPPUNIT_ASSERT(pEditShell);
    SwTextAttr const*const pAttr(pEditShell->GetCursor()->GetPointNode().GetTextNode()->GetTextAttrForCharAt(0, RES_TXTATR_FLYCNT));
    CPPUNIT_ASSERT(pAttr);
    SwFrameFormat const*const pFmt(pAttr->GetFlyCnt().GetFrameFormat());
    SwFormatFrameSize const& rSize(pFmt->GetFormatAttr(RES_FRM_SIZE));
    CPPUNIT_ASSERT_EQUAL(Size(1835, 560), rSize.GetSize());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageLazyRead)
{
    createSwWebDoc("image-lazy-read.html");
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), u"Graphic"_ustr);
    Graphic aGraphic(xGraphic);
    // This failed, import loaded the graphic, it wasn't lazy-read.
    CPPUNIT_ASSERT(!aGraphic.isAvailable());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImgDataAttributeIsLink)
{
    // An img with a data attribute is imported as a link to its src, like any other img.
    createSwWebDoc("img-data-attribute.html");
    SwDoc* pDoc = getSwDoc();
    SwNodeOffset nNodes = pDoc->GetNodes().Count();
    for (SwNodeOffset i(0); i < nNodes; ++i)
    {
        if (SwGrfNode* pGrfNode = pDoc->GetNodes()[i]->GetGrfNode())
        {
            CPPUNIT_ASSERT(pGrfNode->IsLinkedFile());
            OUString aFileName;
            pGrfNode->GetFileFilterNms(&aFileName, nullptr);
            CPPUNIT_ASSERT_EQUAL(createFileURL(u"img-data-attribute.png"), aFileName);
            return;
        }
    }
    CPPUNIT_FAIL("no graphic node in the document");
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testChangedby)
{
    createSwWebDoc("meta-changedby.html");
    SwDocShell* pDocShell(getSwDocShell());
    uno::Reference<document::XDocumentProperties> xDocProps;

    CPPUNIT_ASSERT(pDocShell);
    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(pDocShell->GetModel(), uno::UNO_QUERY);
    xDocProps.set(xDPS->getDocumentProperties());

    // get the document properties
    CPPUNIT_ASSERT(xDocProps.is());

    // the doc's property ModifiedBy is set correctly, ...
    CPPUNIT_ASSERT_EQUAL(u"Blah"_ustr, xDocProps->getModifiedBy());

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // ...but there is no comment 'HTML: <meta name="changedby" content="Blah">'
    CPPUNIT_ASSERT(!xFields->hasMoreElements());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTableBorder1px)
{
    createSwWebDoc("table_border_1px.html");
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    table::BorderLine2 aBorder;

    uno::Reference<text::XTextRange> xCellA1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellA1, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell top border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA1, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA1, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell left border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA1, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellB1(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellB1, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell top border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellB1, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB1, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell left border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB1, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellA2(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellA2, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell top border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA2, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellA2, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell left border", aBorder.InnerLineWidth > 0);
    aBorder = getProperty<table::BorderLine2>(xCellA2,u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);

    uno::Reference<text::XTextRange> xCellB2(xTable->getCellByName(u"B2"_ustr), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xCellB2, u"TopBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell top border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell bottom border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unexpected cell left border", sal_Int16(0), aBorder.InnerLineWidth);
    aBorder = getProperty<table::BorderLine2>(xCellB2, u"RightBorder"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Missing cell right border", aBorder.InnerLineWidth > 0);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testOutlineLevel)
{
    createSwWebDoc("outline-level.html");
    // This was 0, HTML imported into Writer lost the outline numbering for
    // Heading 1 styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(1), u"OutlineLevel"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfBr)
{
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-br.xhtml");
    // <reqif-xhtml:br/> was not recognized as a line break from a ReqIf file.
    CPPUNIT_ASSERT(getParagraph(1)->getString().startsWith("aaa\nbbb"));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf80194_subscript)
{
    createSwWebDoc("tdf80194_subscript.html");
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), u"CharEscapement"_ustr), 0);
    // Most recently, the default subscript was 33%, which is much too large for a subscript.
    // The original 8% (derived from a mathematical calculation) is much better in general,
    // and for HTML was a better match when testing with firefox.
    // DFLT_ESC_AUTO_SUB was tested, but HTML specs are pretty loose, and generally
    // it exceeds the font ascent - so the formula-based-escapement is not appropriate.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -8.f, getProperty<float>(getRun(xPara, 2, u"p"_ustr), u"CharEscapement"_ustr), 1);

    xPara.set(getParagraph(2));
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), u"CharEscapement"_ustr), 0);
    uno::Reference<text::XTextRange> xRun (getRun(xPara, 2, u"L"_ustr));
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 33.f, getProperty<float>(xRun, u"CharEscapement"_ustr), 1);
    // HTML (although unspecified) tends to use a fairly large font. Definitely more than DFLT_ESC_PROP.
    CPPUNIT_ASSERT( 70 < getProperty<sal_Int8>(xRun, u"CharEscapementHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfTable)
{
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-table.xhtml");
    // to see this: soffice --infilter="HTML (StarWriter):xhtmlns=reqif-xhtml" sw/qa/extras/htmlimport/data/reqif-table.xhtml
    // Load a table with xhtmlns=reqif-xhtml filter param.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), xTables->getCount());
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    auto aBorder = getProperty<table::BorderLine2>(xCell, u"TopBorder"_ustr);
    // This was 0, tables had no borders, even if the default autoformat has
    // borders and the markup allows no custom borders.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Top Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Left Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr);
    // This was 0. Single column tables had no right border.  tdf#115576
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Right Border", static_cast<sal_uInt32>(18), aBorder.LineWidth);

    // Test a cell merged horizontally to the table end
    xTable.set(xTables->getByIndex(2), uno::UNO_QUERY_THROW);
    xCell.set(xTable->getCellByName(u"B2"_ustr), uno::UNO_QUERY_THROW);
    aBorder = getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(18), aBorder.LineWidth);
    aBorder = getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr);
    // This was 0, merged cell had no right border.
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(18), aBorder.LineWidth);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageSize)
{
    createSwWebDoc("image-size.html");

    // The markup declares no size, so the frame gets the HTML default size of 2cm by 1cm.  This was
    // 5292 at 96 pixels per inch, i.e. the image's own size, read from the header of the image
    // file while the markup was being parsed.
    awt::Size aSize = getShape(1)->getSize();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2000), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1000), aSize.Height);

    // The image is a link, and its own size reaches the frame when the links are updated.  The
    // image is 200 by 400 pixels and declares 89 pixels per inch, so it is 5.708cm by 11.416cm.
    uno::Reference<util::XLinkUpdate> xLinkUpdate(mxComponent, uno::UNO_QUERY_THROW);
    xLinkUpdate->updateLinks();

    aSize = getShape(1)->getSize();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5708), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11416), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageSizeLinksUpdated)
{
    // When the links are updated as part of the load, the image size is already there when the
    // load returns, without a separate update step.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(document::UpdateDocMode::QUIET_UPDATE)),
    };
    loadWithParams(createFileURL(u"image-size.html"), aParams);

    awt::Size aSize = getShape(1)->getSize();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(5708), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(11416), aSize.Height);
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageNotFetchedWithoutPermission)
{
    // The image an img points at lives outside the document, so nothing may open it while the
    // links are not allowed to update.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"image-size.html"), aParams);

    // the frame has the HTML default size of 2cm by 1cm.  This was 5292 by 10583, the size of the
    // image, which only the image file itself can say
    awt::Size aSize = getShape(1)->getSize();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2000), aSize.Width);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1000), aSize.Height);

    SwDoc* pDoc = getSwDoc();
    SwNodeOffset nNodes = pDoc->GetNodes().Count();
    for (SwNodeOffset i(0); i < nNodes; ++i)
    {
        if (SwGrfNode* pGrfNode = pDoc->GetNodes()[i]->GetGrfNode())
        {
            // and the image itself never arrived
            CPPUNIT_ASSERT(pGrfNode->GetGrfObj().GetType() == GraphicType::NONE
                           || pGrfNode->GetGrfObj().GetType() == GraphicType::Default);
            return;
        }
    }
    CPPUNIT_FAIL("no graphic node in the document");
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf45307RepeatedClipboardImage)
{
    ScopedConfigValue<officecfg::Office::Security::Net::AllowInsecureProtocols>
        aAllowInsecureProtocols(true);
    createSwDoc();
    ScopedWebDAVHTTPProvider aHTTPProvider;

    SvFileStream aImageStream(createFileURL(u"libreoffice.jpg"), StreamMode::READ);
    CPPUNIT_ASSERT(aImageStream.IsOpen());
    const sal_Int32 nImageSize = static_cast<sal_Int32>(aImageStream.remainingSize());
    CPPUNIT_ASSERT(nImageSize > 0);
    uno::Sequence<sal_Int8> aImageBytes(nImageSize);
    aImageStream.ReadBytes(aImageBytes.getArray(), nImageSize);
    const OString aImageData(reinterpret_cast<const char*>(aImageBytes.getConstArray()),
                             nImageSize);

    LocalImageServer aServer(aImageData);
    CPPUNIT_ASSERT(aServer.start());

    const OString aBaseURL
        = OString::Concat("http://127.0.0.1:") + OString::number(aServer.getPort());
    const OString aHTMLStart = OString::Concat("<html><body><table><tr><td><img src=\"") + aBaseURL
                               + "/single.jpg\"></td>";
    OStringBuffer aHTML(aHTMLStart);
    for (int i = 0; i < 3; ++i)
        aHTML.append(OString::Concat("<td><img src=\"") + aBaseURL + "/repeated.jpg\"></td>");
    aHTML.append("</tr></table></body></html>");

    class HTMLTransferable final : public comphelper::WeakImplHelper<datatransfer::XTransferable>
    {
    public:
        explicit HTMLTransferable(OString aHTML)
            : m_aHTML(std::move(aHTML))
        {
        }

        uno::Any getTransferData(const datatransfer::DataFlavor& rFlavor) override
        {
            if (!isDataFlavorSupported(rFlavor))
                return {};

            uno::Sequence<sal_Int8> aBytes(m_aHTML.getLength());
            std::copy_n(reinterpret_cast<const sal_Int8*>(m_aHTML.getStr()), m_aHTML.getLength(),
                        aBytes.getArray());
            return uno::Any(aBytes);
        }

        uno::Sequence<datatransfer::DataFlavor> getTransferDataFlavors() override
        {
            return { getHTMLFlavor() };
        }

        bool isDataFlavorSupported(const datatransfer::DataFlavor& rFlavor) override
        {
            return rFlavor.MimeType.equalsIgnoreAsciiCase(getHTMLFlavor().MimeType);
        }

    private:
        OString m_aHTML;

        static datatransfer::DataFlavor getHTMLFlavor()
        {
            datatransfer::DataFlavor aFlavor;
            CPPUNIT_ASSERT(SotExchange::GetFormatDataFlavor(SotClipboardFormatId::HTML, aFlavor));
            return aFlavor;
        }
    };

    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    uno::Reference<datatransfer::XTransferable> xTransferable
        = new HTMLTransferable(aHTML.makeStringAndClear());
    TransferableDataHelper aDataHelper(xTransferable);
    CPPUNIT_ASSERT(aDataHelper.HasFormat(SotClipboardFormatId::HTML));
    // PasteFileContent gives SwHTMLParser an empty base URL for clipboard HTML.
    CPPUNIT_ASSERT(
        SwTransferable::PasteFormat(*pWrtShell, aDataHelper, SotClipboardFormatId::HTML));

    SwDoc* pDoc = getSwDoc();
    sal_Int32 nGraphics = 0;
    for (SwNodeOffset nNode(0); nNode < pDoc->GetNodes().Count(); ++nNode)
    {
        if (SwGrfNode* pGrfNode = pDoc->GetNodes()[nNode]->GetGrfNode())
        {
            ++nGraphics;
            CPPUNIT_ASSERT_EQUAL(GraphicType::Bitmap, pGrfNode->GetGrfObj().GetType());
        }
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), nGraphics);

    aServer.stop();
    CPPUNIT_ASSERT_MESSAGE(aServer.getRequestLog(), aServer.getConnectionCount() > 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aServer.getRequestLog(), sal_Int32(1),
                                 aServer.getSingleRequestCount());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(aServer.getRequestLog(), sal_Int32(1),
                                 aServer.getRepeatedRequestCount());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testImageButtonPictureWaitsForLinkUpdate)
{
    // An image button's picture is set only once the links are allowed to update.
    cpo::uno::Sequence<beans::PropertyValue> aParams = {
        comphelper::makePropertyValue(u"UpdateDocMode"_ustr,
                                      sal_Int16(document::UpdateDocMode::NO_UPDATE)),
    };
    loadWithParams(createFileURL(u"image-button.html"), aParams);

    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xButton(xControlShape->getControl(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(xButton, u"ImageURL"_ustr));

    uno::Reference<util::XLinkUpdate> xLinkUpdate(mxComponent, uno::UNO_QUERY_THROW);
    xLinkUpdate->updateLinks();

    CPPUNIT_ASSERT_EQUAL(createFileURL(u"image-button.png"),
                         getProperty<OUString>(xButton, u"ImageURL"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf79298StrikeoutVariants)
{
    createSwWebDoc("tdf79298-strikeout-variants.html");

    // Without the accompanying fix in place, this tests would have failed with:
    // - Expected: 1 (FontStrikeout::SINGLE)
    // - Actual  : 0 (FontStrikeout::NONE)
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Strikeout for <del> missing", sal_Int16(awt::FontStrikeout::SINGLE),
        getProperty<sal_Int16>(getRun(getParagraph(1), 1), u"CharStrikeout"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Strikeout for <s> missing", sal_Int16(awt::FontStrikeout::SINGLE),
        getProperty<sal_Int16>(getRun(getParagraph(2), 1), u"CharStrikeout"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Strikeout for <strike> missing", sal_Int16(awt::FontStrikeout::SINGLE),
        getProperty<sal_Int16>(getRun(getParagraph(3), 1), u"CharStrikeout"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf132770InsertedText)
{
    createSwWebDoc("tdf132770-inserted-text.html");

    // Without the accompanying fix in place, this tests would have failed with:
    // - Expected: 1 (FontLineStyle::LINESTYLE_SINGLE)
    // - Actual  : 0 (FontLineStyle::NONE)
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Underline for <ins> missing", sal_Int16(awt::FontUnderline::SINGLE),
        getProperty<sal_Int16>(getRun(getParagraph(1), 1), u"CharUnderline"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf142781)
{
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;
    createSwWebDoc("tdf142781.html");
    OutputDevice* pDevice = Application::GetDefaultDevice();
    Size aPixelSize(672, 480);
    Size aExpected = pDevice->PixelToLogic(aPixelSize, MapMode(MapUnit::Map100thMM));
    awt::Size aSize = getShape(1)->getSize();
    // Without the fix in place, this test would have failed with
    // - Expected: 12700
    // - Actual: 25400
   // CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(aExpected.getHeight()), aSize.Height);
    aSize = getShape(2)->getSize();
    // Without the fix in place, this test would have failed with
    // - Expected: 17780
    // - Actual: 35560
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(aExpected.getWidth()), aSize.Width);
}


CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf122789)
{
    createSwWebDoc("tdf122789.html");
    SwDoc* pDoc = getSwDoc();
    const auto& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFormats.size());
    // This failed, the image had an absolute size, not a relative one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(70), rFormats[0]->GetAttrSet().GetFrameSize().GetWidthPercent());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf118579)
{
    createSwWebDoc("tdf118579.html");
    //Without the fix in place, the file fails to load
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testReqIfPageStyle)
{
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("reqif-page-style.xhtml");
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: Standard, Actual  : HTML'.
    CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr,
                         getProperty<OUString>(getParagraph(1), u"PageStyleName"_ustr));
}

/// HTML import to the sw doc model tests.
class SwHtmlOptionsImportTest : public SwModelTestBase
{
    public:
        SwHtmlOptionsImportTest() : SwModelTestBase(u"/sw/qa/extras/htmlimport/data/"_ustr) {}
};

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testAllowedRTFOLEMimeTypes)
{
    cpo::uno::Sequence<OUString> aTypes = { u"test/rtf"_ustr };
    cpo::uno::Sequence<beans::PropertyValue> aLoadProperties = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"HTML (StarWriter)"_ustr),
        comphelper::makePropertyValue(u"FilterOptions"_ustr, u"xhtmlns=reqif-xhtml"_ustr),
        comphelper::makePropertyValue(u"AllowedRTFOLEMimeTypes"_ustr, aTypes),
    };
    OUString aURL = createFileURL(u"allowed-rtf-ole-mime-types.xhtml");
    loadWithParams(aURL, aLoadProperties);
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
    uno::Reference<document::XEmbeddedObjectSupplier2> xObject(xObjects->getByIndex(0),
                                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT(xObject.is());
    uno::Reference<embed::XInplaceObject> xEmbeddedObject(
        xObject->getExtendedControlOverEmbeddedObject(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed, because the returned
    // embedded object was a dummy one, which does not support in-place editing.
    CPPUNIT_ASSERT(xEmbeddedObject.is());
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testHiddenTextframe)
{
    // Load HTML content into Writer, similar to HTML paste.
    createSwWebDoc("hidden-textframe.html");

    // Check the content of the draw page.
    uno::Reference<drawing::XDrawPageSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xSupplier->getDrawPage();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. an unexpected text frame was created, covering the actual content.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testOleImg)
{
    // Given an XHTML with an <object> (containing GIF) and an inner <object> (containing PNG, to be
    // ignored):
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("ole-img.xhtml");

    // Then make sure the result is a single Writer image:
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getGraphicObjects(),
                                                     uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the image was not imported as a Writer image (but as an OLE object).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testOleImgSvg)
{
    // Given an XHTML with an <object> (containing SVG) and an inner <object> (containing PNG, to be
    // ignored):
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("ole-img-svg.xhtml");

    // Then make sure the result is a single Writer image:
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getGraphicObjects(),
                                                     uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the image was not imported as a Writer image (but as an OLE object).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testUTF16_nonBMP)
{
    createSwWebDoc("emojis16BE.html");
    // tdf#146173: non-BMP characters' surrogates didn't combine correctly
    CPPUNIT_ASSERT_EQUAL(u"a text with emojis: 🌾 ☀👨🏼‍🌾🏃🏼‍♂️🤙🏽🔍"_ustr,
                         getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf154273)
{
    createSwWebDoc("tdf154273.html");

    // Without the fix in place, this test would have failed with
    // - Expected: 'test'
    // - Actual  : &apos;test&apos;
    CPPUNIT_ASSERT_EQUAL(u"'test' "_ustr, getParagraph(1)->getString());
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testOleData)
{
    // Given an XHTML with an <object> (containing non-image, non-OLE2 data) and an inner <object>
    // (containing PNG):
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("ole-data.xhtml");

    // Then make sure the result is a single clickable Writer image:
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getGraphicObjects(),
                                                     uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the image was not imported as a Writer image (but as an OLE object).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());
    uno::Reference<css::drawing::XShape> xShape = getShape(1);
    // And then the image was not clickable: this was empty.
    CPPUNIT_ASSERT(getProperty<OUString>(xShape, u"HyperLinkURL"_ustr).endsWith("/data.ole"));
}

CPPUNIT_TEST_FIXTURE(SwHtmlOptionsImportTest, testOleData2)
{
    // Given an XHTML with 2 objects: the first has a link, the second does not have:
    setImportFilterOptions(u"xhtmlns=reqif-xhtml"_ustr);
    setImportFilterName(TestFilter::HTML_WRITER);
    createSwDoc("ole-data2.xhtml");

    // Then make sure that the second image doesn't have a link set:
    uno::Reference<text::XTextGraphicObjectsSupplier> xSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xSupplier->getGraphicObjects(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xObjects->getCount());
    uno::Reference<css::drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT(getProperty<OUString>(xShape, u"HyperLinkURL"_ustr).endsWith("/data.ole"));
    xShape = getShape(2);
    // Without the accompanying fix in place, this test would have failed, the link from the 1st
    // image leaked to the 2nd image.
    CPPUNIT_ASSERT(getProperty<OUString>(xShape, u"HyperLinkURL"_ustr).isEmpty());
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testRGBAColor)
{
    createSwWebDoc("green-highlight.html");
    const uno::Reference<text::XTextRange> xPara = getParagraph(1);
    const uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
    const Color nBackColor(0xaed89a);

    // Without the accompanying fix in place, this test would have failed, the background
    // color was not imported at all, when it was in hex RGBA format in HTML.
    CPPUNIT_ASSERT_EQUAL(nBackColor, getProperty<Color>(xRun, u"CharBackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf154581)
{
    createSwWebDoc("tdf154581.html");

    // Without the fix in place, this test would have failed with
    // - Expected: 150
    // - Actual  : 100
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(awt::FontUnderline::SINGLE), getProperty<sal_Int16>(getRun(getParagraph(2), 1), u"CharUnderline"_ustr));
    CPPUNIT_ASSERT_EQUAL(awt::FontSlant_ITALIC, getProperty<awt::FontSlant>(getRun(getParagraph(3), 1), u"CharPosture"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf153341)
{
    createSwWebDoc("tdf153341.html");

    const uno::Reference<text::XTextRange> xPara1 = getParagraph(1);
    const uno::Reference<beans::XPropertySet> xRun1(getRun(xPara1,1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x00, 0xFF, 0x00, 0x00), getProperty<Color>(xRun1, u"CharColor"_ustr));

    const uno::Reference<text::XTextRange> xPara2 = getParagraph(2);
    const uno::Reference<beans::XPropertySet> xRun2(getRun(xPara2,1), uno::UNO_QUERY);

    // Without the fix in place, this test would have failed with
    // - Expected: rgba[ff00007f]
    // - Actual  : rgba[ff0000ff]
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0x80, 0xFF, 0x00, 0x00), getProperty<Color>(xRun2, u"CharColor"_ustr));

    const uno::Reference<text::XTextRange> xPara3 = getParagraph(3);
    const uno::Reference<beans::XPropertySet> xRun3(getRun(xPara3,1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(ColorTransparency, 0xB3, 0xFF, 0x00, 0x00), getProperty<Color>(xRun3, u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testTdf155011)
{
    createSwWebDoc("tdf155011.html");
    // Must not crash / fail asserts

    // tdf#145689: don't lose page style when inserting table-in-table
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"HTML"_ustr, getProperty<OUString>(xCursor, u"PageStyleName"_ustr));
}

CPPUNIT_TEST_FIXTURE(HtmlImportTest, testNestedListMixedType)
{
    createSwWebDoc("nested-list-mixed-type.html");
    // The HTML has an outer <ol> with two <li>s:
    //   first <li> contains a nested <ol>
    //   second <li> contains a nested <ul>
    // Without the fix, the nested <ul> was imported as <ol> (inheriting the sibling's type).

    // Find paragraphs "Ordered 1" and "Bullet 1" and check their numbering types.
    // "Ordered 1" should be SVX_NUM_ARABIC (ordered list)
    // "Bullet 1" should be SVX_NUM_CHAR_SPECIAL (unordered/bullet list)
    bool bFoundOrdered = false;
    bool bFoundBullet = false;
    uno::Reference<text::XTextDocument> xTextDoc(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaAccess(xTextDoc->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
        OUString sText = xPara->getString().trim();
        uno::Reference<beans::XPropertySet> xProps(xPara, uno::UNO_QUERY);
        bool bIsNumber = false;
        xProps->getPropertyValue(u"NumberingIsNumber"_ustr) >>= bIsNumber;
        if (!bIsNumber)
            continue;

        uno::Reference<container::XIndexAccess> xLevels(
            xProps->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
        sal_Int16 nLevel = -1;
        xProps->getPropertyValue(u"NumberingLevel"_ustr) >>= nLevel;
        if (nLevel < 0)
            continue;

        cpo::uno::Sequence<beans::PropertyValue> aRuleProps;
        xLevels->getByIndex(nLevel) >>= aRuleProps;
        sal_Int16 nNumberingType = -1;
        for (const auto& rProp : aRuleProps)
        {
            if (rProp.Name == "NumberingType")
            {
                nNumberingType = rProp.Value.get<sal_Int16>();
                break;
            }
        }

        if (sText.indexOf("Ordered 1") >= 0)
        {
            CPPUNIT_ASSERT_EQUAL(style::NumberingType::ARABIC, nNumberingType);
            bFoundOrdered = true;
        }
        else if (sText.indexOf("Bullet 1") >= 0)
        {
            CPPUNIT_ASSERT_EQUAL(style::NumberingType::CHAR_SPECIAL, nNumberingType);
            bFoundBullet = true;
        }
    }
    CPPUNIT_ASSERT_MESSAGE("paragraph 'Ordered 1' not found", bFoundOrdered);
    CPPUNIT_ASSERT_MESSAGE("paragraph 'Bullet 1' not found", bFoundBullet);
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
