/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <sfx2/app.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <IDocumentLayoutAccess.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <unotxdoc.hxx>
#include <viewsh.hxx>

using namespace css;

void SwModelTestBase::paste(std::u16string_view aFilename,
                            uno::Reference<text::XTextRange> const& xTextRange)
{
    uno::Reference<document::XFilter> xFilter(
        m_xSFactory->createInstance("com.sun.star.comp.Writer.RtfFilter"), uno::UNO_QUERY_THROW);
    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
    xImporter->setTargetDocument(mxComponent);
    uno::Sequence<beans::PropertyValue> aDescriptor(3);
    aDescriptor[0].Name = "InputStream";
    std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(
        m_directories.getURLFromSrc(u"/sw/qa/extras/") + aFilename, StreamMode::STD_READ);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE, pStream->GetError());
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
    aDescriptor[0].Value <<= xStream;
    aDescriptor[1].Name = "InsertMode";
    aDescriptor[1].Value <<= true;
    aDescriptor[2].Name = "TextInsertModeRange";
    aDescriptor[2].Value <<= xTextRange;
    CPPUNIT_ASSERT(xFilter->filter(aDescriptor));
}

SwModelTestBase::SwModelTestBase(const OUString& pTestDocumentPath, const char* pFilter)
    : mpXmlBuffer(nullptr)
    , mpTestDocumentPath(pTestDocumentPath)
    , mpFilter(pFilter)
    , mnStartTime(0)
    , mbExported(false)
{
    maTempFile.EnableKillingFile();
}

void SwModelTestBase::setUp()
{
    test::BootstrapFixture::setUp();
    mxDesktop.set(
        css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
    SfxApplication::GetOrCreate();
}

void SwModelTestBase::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void SwModelTestBase::executeImportTest(const char* filename, const char* pPassword)
{
    // If the testcase is stored in some other format, it's pointless to test.
    if (mustTestImportOf(filename))
    {
        maTempFile.EnableKillingFile(false);
        header();
        std::unique_ptr<Resetter> const pChanges(preTest(filename));
        load(mpTestDocumentPath, filename, pPassword);
        verify();
        finish();
        maTempFile.EnableKillingFile();
    }
}

void SwModelTestBase::executeLoadVerifyReloadVerify(const char* filename, const char* pPassword)
{
    maTempFile.EnableKillingFile(false);
    header();
    std::unique_ptr<Resetter> const pChanges(preTest(filename));
    load(mpTestDocumentPath, filename, pPassword);
    if (mustTestImportOf(filename))
    {
        verify();
    }
    postLoad(filename);
    reload(mpFilter, filename, pPassword);
    verify();
    finish();
    maTempFile.EnableKillingFile();
}

void SwModelTestBase::executeLoadReloadVerify(const char* filename, const char* pPassword)
{
    maTempFile.EnableKillingFile(false);
    header();
    std::unique_ptr<Resetter> const pChanges(preTest(filename));
    load(mpTestDocumentPath, filename, pPassword);
    postLoad(filename);
    reload(mpFilter, filename, pPassword);
    verify();
    finish();
    maTempFile.EnableKillingFile();
}

void SwModelTestBase::executeImportExport(const char* filename, const char* pPassword)
{
    maTempFile.EnableKillingFile(false);
    header();
    std::unique_ptr<Resetter> const pChanges(preTest(filename));
    load(mpTestDocumentPath, filename, pPassword);
    save(OUString::createFromAscii(mpFilter), maTempFile);
    maTempFile.EnableKillingFile(false);
    verify();
    finish();
    maTempFile.EnableKillingFile();
}

void SwModelTestBase::dumpLayout(const uno::Reference<lang::XComponent>& rComponent)
{
    // create the xml writer
    mpXmlBuffer = xmlBufferCreate();
    xmlTextWriterPtr pXmlWriter = xmlNewTextWriterMemory(mpXmlBuffer, 0);
    xmlTextWriterStartDocument(pXmlWriter, nullptr, nullptr, nullptr);

    // create the dump
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(rComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    pLayout->dumpAsXml(pXmlWriter);

    // delete xml writer
    xmlTextWriterEndDocument(pXmlWriter);
    xmlFreeTextWriter(pXmlWriter);
}

void SwModelTestBase::discardDumpedLayout()
{
    if (mpXmlBuffer)
    {
        xmlBufferFree(mpXmlBuffer);
        mpXmlBuffer = nullptr;
    }
}

void SwModelTestBase::calcLayout()
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    pDoc->getIDocumentLayoutAccess().GetCurrentViewShell()->CalcLayout();
}

int SwModelTestBase::getLength() const
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    OUStringBuffer aBuf;
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(),
                                                                       uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRange(xRangeEnum->nextElement(), uno::UNO_QUERY);
            aBuf.append(xRange->getString());
        }
    }
    return aBuf.getLength();
}

uno::Reference<container::XNameAccess> SwModelTestBase::getStyles(const OUString& aFamily)
{
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(aFamily),
                                                        uno::UNO_QUERY);
    return xStyleFamily;
}

uno::Reference<style::XAutoStyleFamily> SwModelTestBase::getAutoStyles(const OUString& aFamily)
{
    uno::Reference<style::XAutoStylesSupplier> xAutoStylesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<style::XAutoStyles> xAutoStyles(xAutoStylesSupplier->getAutoStyles());
    uno::Reference<style::XAutoStyleFamily> xAutoStyleFamily(xAutoStyles->getByName(aFamily),
                                                             uno::UNO_QUERY);
    return xAutoStyleFamily;
}

xmlDocUniquePtr SwModelTestBase::parseLayoutDump()
{
    if (!mpXmlBuffer)
        dumpLayout(mxComponent);

    return xmlDocUniquePtr(
        xmlParseMemory(reinterpret_cast<const char*>(xmlBufferContent(mpXmlBuffer)),
                       xmlBufferLength(mpXmlBuffer)));
}

OUString SwModelTestBase::parseDump(const OString& aXPath, const OString& aAttribute)
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc.get());
    xmlXPathObjectPtr pXmlXpathObj
        = xmlXPathEvalExpression(BAD_CAST(aXPath.getStr()), pXmlXpathCtx);
    CPPUNIT_ASSERT_MESSAGE("xpath evaluation failed", pXmlXpathObj);
    xmlChar* pXpathStrResult;
    if (pXmlXpathObj->type == XPATH_NODESET)
    {
        xmlNodeSetPtr pXmlNodes = pXmlXpathObj->nodesetval;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("xpath should match exactly 1 node", 1,
                                     xmlXPathNodeSetGetLength(pXmlNodes));
        xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
        if (aAttribute.getLength())
            pXpathStrResult = xmlGetProp(pXmlNode, BAD_CAST(aAttribute.getStr()));
        else
            pXpathStrResult = xmlNodeGetContent(pXmlNode);
    }
    else
    {
        // the xpath expression evaluated to a value, not a node
        CPPUNIT_ASSERT_EQUAL_MESSAGE("attr name should not be supplied when xpath evals to a value",
                                     sal_Int32(0), aAttribute.getLength());
        pXpathStrResult = xmlXPathCastToString(pXmlXpathObj);
        CPPUNIT_ASSERT_MESSAGE("xpath result cannot be cast to string", pXpathStrResult);
    }

    OUString aRet(reinterpret_cast<char*>(pXpathStrResult), xmlStrlen(pXpathStrResult),
                  RTL_TEXTENCODING_UTF8);
    xmlFree(pXpathStrResult);
    xmlFree(pXmlXpathObj);
    xmlFree(pXmlXpathCtx);

    return aRet;
}

bool SwModelTestBase::hasProperty(const uno::Reference<uno::XInterface>& obj,
                                  const OUString& name) const
{
    uno::Reference<beans::XPropertySet> properties(obj, uno::UNO_QUERY_THROW);
    return properties->getPropertySetInfo()->hasPropertyByName(name);
}

xml::AttributeData SwModelTestBase::getUserDefineAttribute(const uno::Any& obj,
                                                           const OUString& name,
                                                           const OUString& rValue) const
{
    uno::Reference<container::XNameContainer> attrsCnt(
        getProperty<uno::Any>(obj, "UserDefinedAttributes"), uno::UNO_QUERY_THROW);

    xml::AttributeData aValue;
    attrsCnt->getByName(name) >>= aValue;
    if (!rValue.isEmpty())
        CPPUNIT_ASSERT_EQUAL_MESSAGE("attribute of cell does not contain expected value", rValue,
                                     aValue.Value);

    return aValue;
}

int SwModelTestBase::getParagraphs(uno::Reference<text::XText> const& xText)
{
    int nRet = 0;
    if (!xText.is())
        return nRet;

    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xText->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        xParaEnum->nextElement();
        nRet++;
    }
    return nRet;
}

int SwModelTestBase::getParagraphs()
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    return getParagraphs(xTextDocument->getText());
}

uno::Reference<text::XTextContent>
SwModelTestBase::getParagraphOrTable(int number, uno::Reference<text::XText> const& xText) const
{
    assert(number != 0); // this thing is 1-based
    uno::Reference<container::XEnumerationAccess> paraEnumAccess;
    if (xText.is())
        paraEnumAccess.set(xText, uno::UNO_QUERY);
    else
    {
        uno::Reference<text::XTextDocument> textDocument(mxComponent, uno::UNO_QUERY);
        paraEnumAccess.set(textDocument->getText(), uno::UNO_QUERY);
    }
    uno::Reference<container::XEnumeration> paraEnum = paraEnumAccess->createEnumeration();
    for (int i = 1; i < number; ++i)
        paraEnum->nextElement();
    uno::Reference<text::XTextContent> const xElem(paraEnum->nextElement(), uno::UNO_QUERY_THROW);
    return xElem;
}

uno::Reference<text::XTextRange> SwModelTestBase::getParagraph(int number,
                                                               const OUString& content) const
{
    uno::Reference<text::XTextRange> const xParagraph(getParagraphOrTable(number),
                                                      uno::UNO_QUERY_THROW);
    if (!content.isEmpty())
        CPPUNIT_ASSERT_EQUAL_MESSAGE("paragraph does not have expected content", content,
                                     xParagraph->getString());
    return xParagraph;
}

sal_Int16 SwModelTestBase::getNumberingTypeOfParagraph(int nPara)
{
    sal_Int16 nNumberingType = -1;
    uno::Reference<text::XTextRange> xPara(getParagraph(nPara));
    uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
    bool isNumber = false;
    properties->getPropertyValue("NumberingIsNumber") >>= isNumber;
    if (isNumber)
    {
        uno::Reference<container::XIndexAccess> xLevels(
            properties->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        sal_Int16 nNumberingLevel = -1;
        properties->getPropertyValue("NumberingLevel") >>= nNumberingLevel;
        if (nNumberingLevel >= 0 && nNumberingLevel < xLevels->getCount())
        {
            uno::Sequence<beans::PropertyValue> aPropertyValue;
            xLevels->getByIndex(nNumberingLevel) >>= aPropertyValue;
            auto pProp = std::find_if(
                aPropertyValue.begin(), aPropertyValue.end(),
                [](const beans::PropertyValue& rProp) { return rProp.Name == "NumberingType"; });
            if (pProp != aPropertyValue.end())
                nNumberingType = pProp->Value.get<sal_Int16>();
        }
    }
    return nNumberingType;
}

uno::Reference<text::XTextRange>
SwModelTestBase::getParagraphOfText(int number, uno::Reference<text::XText> const& xText,
                                    const OUString& content) const
{
    uno::Reference<text::XTextRange> const xParagraph(getParagraphOrTable(number, xText),
                                                      uno::UNO_QUERY_THROW);
    if (!content.isEmpty())
        CPPUNIT_ASSERT_EQUAL_MESSAGE("paragraph does not contain expected content", content,
                                     xParagraph->getString());
    return xParagraph;
}

uno::Reference<beans::XPropertySet>
SwModelTestBase::getParagraphAnchoredObject(int const index,
                                            uno::Reference<text::XTextRange> const& xPara) const
{
    uno::Reference<container::XContentEnumerationAccess> xContentEnumAccess(xPara, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum
        = xContentEnumAccess->createContentEnumeration("com.sun.star.text.TextContent");
    for (int i = 1; i < index; ++i)
    {
        xContentEnum->nextElement();
    }
    return uno::Reference<beans::XPropertySet>(xContentEnum->nextElement(), uno::UNO_QUERY);
}

uno::Reference<text::XTextRange>
SwModelTestBase::getRun(uno::Reference<text::XTextRange> const& xParagraph, int number,
                        const OUString& content) const
{
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParagraph, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    for (int i = 1; i < number; ++i)
        xRunEnum->nextElement();
    uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    if (!content.isEmpty())
        CPPUNIT_ASSERT_EQUAL_MESSAGE("run does not contain expected content", content,
                                     xRun->getString());
    return xRun;
}

OUString SwModelTestBase::getFormula(uno::Reference<text::XTextRange> const& xRun) const
{
    uno::Reference<container::XContentEnumerationAccess> xContentEnumAccess(xRun, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xContentEnum
        = xContentEnumAccess->createContentEnumeration("");
    uno::Reference<beans::XPropertySet> xFormula(xContentEnum->nextElement(), uno::UNO_QUERY);
    return getProperty<OUString>(
        getProperty<uno::Reference<beans::XPropertySet>>(xFormula, "Model"), "Formula");
}

uno::Reference<table::XCell>
SwModelTestBase::getCell(uno::Reference<uno::XInterface> const& xTableIfc, OUString const& rCell,
                         OUString const& rContent)
{
    uno::Reference<text::XTextTable> const xTable(xTableIfc, uno::UNO_QUERY_THROW);
    uno::Reference<table::XCell> const xCell(xTable->getCellByName(rCell), uno::UNO_SET_THROW);
    if (!rContent.isEmpty())
    {
        uno::Reference<text::XText> const xCellText(xCell, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cell does not contain expected content", rContent,
                                     xCellText->getString());
    }
    return xCell;
}

uno::Reference<drawing::XShape> SwModelTestBase::getShape(int number)
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(number - 1), uno::UNO_QUERY);
    return xShape;
}

uno::Reference<drawing::XShape> SwModelTestBase::getShapeByName(std::u16string_view aName)
{
    uno::Reference<drawing::XShape> xRet;

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    for (sal_Int32 i = 0; i < xDrawPage->getCount(); ++i)
    {
        uno::Reference<container::XNamed> xShape(xDrawPage->getByIndex(i), uno::UNO_QUERY);
        if (xShape->getName() == aName)
        {
            xRet.set(xShape, uno::UNO_QUERY);
            break;
        }
    }

    return xRet;
}

uno::Reference<drawing::XShape> SwModelTestBase::getTextFrameByName(const OUString& aName)
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNameAccess = xTextFramesSupplier->getTextFrames();
    uno::Reference<drawing::XShape> xShape(xNameAccess->getByName(aName), uno::UNO_QUERY);
    return xShape;
}

void SwModelTestBase::setTestInteractionHandler(const char* pPassword,
                                                std::vector<beans::PropertyValue>& rFilterOptions)
{
    OUString sPassword = OUString::createFromAscii(pPassword);
    rFilterOptions.emplace_back();
    xInteractionHandler
        = rtl::Reference<TestInteractionHandler>(new TestInteractionHandler(sPassword));
    uno::Reference<task::XInteractionHandler2> const xInteraction(xInteractionHandler);
    rFilterOptions[0].Name = "InteractionHandler";
    rFilterOptions[0].Value <<= xInteraction;
}

void SwModelTestBase::header() {}

void SwModelTestBase::loadURL(OUString const& rURL, const char* pName, const char* pPassword)
{
    if (mxComponent.is())
        mxComponent->dispose();

    std::vector<beans::PropertyValue> aFilterOptions;

    if (pPassword)
    {
        setTestInteractionHandler(pPassword, aFilterOptions);
    }

    if (!maImportFilterOptions.isEmpty())
    {
        beans::PropertyValue aValue;
        aValue.Name = "FilterOptions";
        aValue.Value <<= maImportFilterOptions;
        aFilterOptions.push_back(aValue);
    }

    if (!maImportFilterName.isEmpty())
    {
        beans::PropertyValue aValue;
        aValue.Name = "FilterName";
        aValue.Value <<= maImportFilterName;
        aFilterOptions.push_back(aValue);
    }

    // Output name early, so in the case of a hang, the name of the hanging input file is visible.
    if (pName)
        std::cout << pName << ":\n";
    mnStartTime = osl_getGlobalTimer();
    mxComponent = loadFromDesktop(rURL, "com.sun.star.text.TextDocument",
                                  comphelper::containerToSequence(aFilterOptions));

    if (pPassword)
    {
        CPPUNIT_ASSERT_MESSAGE("Password set but not requested",
                               xInteractionHandler->wasPasswordRequested());
    }

    discardDumpedLayout();
    if (pName && mustCalcLayoutOf(pName))
        calcLayout();
}

void SwModelTestBase::reload(const char* pFilter, const char* filename, const char* pPassword)
{
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    OUString aFilterName = OUString::createFromAscii(pFilter);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= aFilterName;
    if (!maFilterOptions.isEmpty())
        aMediaDescriptor["FilterOptions"] <<= maFilterOptions;
    if (pPassword)
    {
        if (strcmp(pFilter, "Office Open XML Text"))
        {
            aMediaDescriptor["Password"] <<= OUString::createFromAscii(pPassword);
        }
        else
        {
            OUString sPassword = OUString::createFromAscii(pPassword);
            css::uno::Sequence<css::beans::NamedValue> aEncryptionData{
                { "CryptoType", css::uno::makeAny(OUString("Standard")) },
                { "OOXPassword", css::uno::makeAny(sPassword) }
            };
            aMediaDescriptor[utl::MediaDescriptor::PROP_ENCRYPTIONDATA()] <<= aEncryptionData;
        }
    }
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    uno::Reference<lang::XComponent> xComponent(xStorable, uno::UNO_QUERY);
    xComponent->dispose();
    mbExported = true;

    std::vector<beans::PropertyValue> aFilterOptions;
    if (pPassword)
    {
        setTestInteractionHandler(pPassword, aFilterOptions);
    }

    if (!maImportFilterOptions.isEmpty())
    {
        beans::PropertyValue aValue;
        aValue.Name = "FilterOptions";
        aValue.Value <<= maImportFilterOptions;
        aFilterOptions.push_back(aValue);
    }

    if (!maImportFilterName.isEmpty())
    {
        beans::PropertyValue aValue;
        aValue.Name = "FilterName";
        aValue.Value <<= maImportFilterName;
        aFilterOptions.push_back(aValue);
    }

    mxComponent = loadFromDesktop(maTempFile.GetURL(), "com.sun.star.text.TextDocument",
                                  comphelper::containerToSequence(aFilterOptions));
    if (pPassword)
    {
        CPPUNIT_ASSERT_MESSAGE("Password set but not requested",
                               xInteractionHandler->wasPasswordRequested());
    }
    if (mustValidate(filename) || aFilterName == "writer8"
        || aFilterName == "OpenDocument Text Flat XML")
    {
        if (aFilterName == "Office Open XML Text")
        {
            // too many validation errors right now
            validate(maTempFile.GetFileName(), test::OOXML);
        }
        else if (aFilterName == "writer8" || aFilterName == "OpenDocument Text Flat XML")
        {
            validate(maTempFile.GetFileName(), test::ODF);
        }
        else if (aFilterName == "MS Word 97")
        {
            validate(maTempFile.GetFileName(), test::MSBINARY);
        }
        else
        {
            OString aMessage
                = OStringLiteral("validation requested, but don't know how to validate ") + filename
                  + " (" + OUStringToOString(aFilterName, RTL_TEXTENCODING_UTF8) + ")";
            CPPUNIT_FAIL(aMessage.getStr());
        }
    }
    discardDumpedLayout();
    if (mustCalcLayoutOf(filename))
        calcLayout();
}

void SwModelTestBase::save(const OUString& aFilterName, utl::TempFile& rTempFile)
{
    rTempFile.EnableKillingFile();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= aFilterName;
    if (!maFilterOptions.isEmpty())
        aMediaDescriptor["FilterOptions"] <<= maFilterOptions;
    xStorable->storeToURL(rTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    // TODO: for now, validate only ODF here
    if (aFilterName == "writer8" || aFilterName == "OpenDocument Text Flat XML")
    {
        validate(rTempFile.GetFileName(), test::ODF);
    }
}

void SwModelTestBase::finish()
{
    sal_uInt32 nEndTime = osl_getGlobalTimer();
    std::cout << (nEndTime - mnStartTime) << std::endl;
    discardDumpedLayout();
}

int SwModelTestBase::getPages() const
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    return xCursor->getPage();
}

int SwModelTestBase::getShapes() const
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws = xDrawPageSupplier->getDrawPage();
    return xDraws->getCount();
}

xmlDocUniquePtr SwModelTestBase::parseExport(const OUString& rStreamName)
{
    if (!mbExported)
        return nullptr;

    return parseExportInternal(maTempFile.GetURL(), rStreamName);
}

xmlDocUniquePtr SwModelTestBase::parseExportedFile()
{
    auto stream(SvFileStream(maTempFile.GetURL(), StreamMode::READ | StreamMode::TEMPORARY));
    return parseXmlStream(&stream);
}

std::unique_ptr<SvStream> SwModelTestBase::parseExportStream(const OUString& url,
                                                             const OUString& rStreamName)
{
    // Read the stream we're interested in.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory),
                                                      url);
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(rStreamName),
                                                  uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInputStream.is());
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    return pStream;
}

xmlDocUniquePtr SwModelTestBase::parseExportInternal(const OUString& url,
                                                     const OUString& rStreamName)
{
    std::unique_ptr<SvStream> pStream(parseExportStream(url, rStreamName));

    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    pXmlDoc->name = reinterpret_cast<char*>(xmlStrdup(
        reinterpret_cast<xmlChar const*>(OUStringToOString(url, RTL_TEXTENCODING_UTF8).getStr())));
    return pXmlDoc;
}

void SwModelTestBase::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    // docx
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w"),
                       BAD_CAST("http://schemas.openxmlformats.org/wordprocessingml/2006/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("v"), BAD_CAST("urn:schemas-microsoft-com:vml"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("mc"),
                       BAD_CAST("http://schemas.openxmlformats.org/markup-compatibility/2006"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("wps"),
        BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingShape"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("wpg"),
        BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingGroup"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("wp"),
        BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("wp14"),
        BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("a"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("pic"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/picture"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("rels"),
                       BAD_CAST("http://schemas.openxmlformats.org/package/2006/relationships"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w14"),
                       BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordml"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("m"),
                       BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/math"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("ContentType"),
                       BAD_CAST("http://schemas.openxmlformats.org/package/2006/content-types"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("lc"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/lockedCanvas"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("cp"),
        BAD_CAST("http://schemas.openxmlformats.org/package/2006/metadata/core-properties"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("extended-properties"),
        BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("custom-properties"),
        BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/custom-properties"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("vt"),
        BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dcterms"), BAD_CAST("http://purl.org/dc/terms/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("a14"),
                       BAD_CAST("http://schemas.microsoft.com/office/drawing/2010/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("c"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/chart"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("o"),
                       BAD_CAST("urn:schemas-microsoft-com:office:office"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w10"),
                       BAD_CAST("urn:schemas-microsoft-com:office:word"));
    // odt
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("office"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:office:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("style"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:style:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("text"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:text:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("table"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:table:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("draw"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("fo"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("config"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:config:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xlink"), BAD_CAST("http://www.w3.org/1999/xlink"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dc"), BAD_CAST("http://purl.org/dc/elements/1.1/"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("meta"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:meta:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("number"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("svg"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("chart"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:chart:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dr3d"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("math"),
                       BAD_CAST("http://www.w3.org/1998/Math/MathML"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("form"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:form:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("script"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:script:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("ooo"),
                       BAD_CAST("http://openoffice.org/2004/office"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("ooow"),
                       BAD_CAST("http://openoffice.org/2004/writer"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("oooc"), BAD_CAST("http://openoffice.org/2004/calc"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dom"),
                       BAD_CAST("http://www.w3.org/2001/xml-events"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xforms"), BAD_CAST("http://www.w3.org/2002/xforms"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xsd"), BAD_CAST("http://www.w3.org/2001/XMLSchema"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xsi"),
                       BAD_CAST("http://www.w3.org/2001/XMLSchema-instance"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("rpt"),
                       BAD_CAST("http://openoffice.org/2005/report"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("of"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:of:1.2"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xhtml"), BAD_CAST("http://www.w3.org/1999/xhtml"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("grddl"),
                       BAD_CAST("http://www.w3.org/2003/g/data-view#"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("officeooo"),
                       BAD_CAST("http://openoffice.org/2009/office"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("tableooo"),
                       BAD_CAST("http://openoffice.org/2009/table"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("drawooo"),
                       BAD_CAST("http://openoffice.org/2010/draw"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("calcext"),
        BAD_CAST("urn:org:documentfoundation:names:experimental:calc:xmlns:calcext:1.0"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("loext"),
        BAD_CAST("urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("field"),
        BAD_CAST("urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0"));
    xmlXPathRegisterNs(
        pXmlXpathCtx, BAD_CAST("formx"),
        BAD_CAST("urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("css3t"),
                       BAD_CAST("http://www.w3.org/TR/css3-text/"));
    // reqif-xhtml
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("reqif-xhtml"),
                       BAD_CAST("http://www.w3.org/1999/xhtml"));
}

SwDoc* SwModelTestBase::createSwDoc(std::u16string_view rDataDirectory, const char* pName)
{
    if (rDataDirectory.empty() || !pName)
        loadURL("private:factory/swriter", nullptr);
    else
        load(rDataDirectory, pName);

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    return pTextDoc->GetDocShell()->GetDoc();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
