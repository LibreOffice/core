/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/xmltesttools.hxx>

#include <memory>

#include <vcl/mtfxmldump.hxx>
#include <sal/log.hxx>

namespace {

OUString convert(xmlChar const * string) {
    OUString s;
    CPPUNIT_ASSERT_MESSAGE(
        "xmlChar string is not UTF-8",
        rtl_convertStringToUString(
            &s.pData, reinterpret_cast<char const *>(string), xmlStrlen(string),
            RTL_TEXTENCODING_UTF8,
            (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
             | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)));
    return s;
}

OString oconvert(xmlChar const * string)
{
    return reinterpret_cast<char const *>(string);
}

}

XmlTestTools::XmlTestTools()
{}

XmlTestTools::~XmlTestTools()
{}

xmlDocUniquePtr XmlTestTools::parseXml(utl::TempFileNamed const & aTempFile)
{
    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    return parseXmlStream(&aFileStream);
}

xmlDocUniquePtr XmlTestTools::parseXmlStream(SvStream* pStream)
{
    std::size_t nSize = pStream->remainingSize();
    std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nSize + 1]);
    pStream->ReadBytes(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;
    auto pCharBuffer = reinterpret_cast<xmlChar*>(pBuffer.get());
    SAL_INFO("test", "XmlTestTools::parseXmlStream: pBuffer is '" << pCharBuffer << "'");
    return xmlDocUniquePtr(xmlParseDoc(pCharBuffer));
}

xmlDocUniquePtr XmlTestTools::dumpAndParse(MetafileXmlDump& rDumper, const GDIMetaFile& rGDIMetaFile)
{
    SvMemoryStream aStream;
    rDumper.dump(rGDIMetaFile, aStream);
    aStream.Seek(STREAM_SEEK_TO_BEGIN);
    return XmlTestTools::parseXmlStream(&aStream);
}

xmlXPathObjectPtr XmlTestTools::getXPathNode(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath)
{
    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc.get());
    registerNamespaces(pXmlXpathCtx);
    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(rXPath.getStr()), pXmlXpathCtx);
    xmlXPathFreeContext(pXmlXpathCtx);
    return pXmlXpathObj;
}

void XmlTestTools::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    // ooxml
    XmlTestTools::registerOOXMLNamespaces(pXmlXpathCtx);
    // odf
    XmlTestTools::registerODFNamespaces(pXmlXpathCtx);
    // reqif-xhtml
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("reqif-xhtml"), BAD_CAST("http://www.w3.org/1999/xhtml"));
}

OUString XmlTestTools::getXPath(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OString& rAttribute)
{
    CPPUNIT_ASSERT(pXmlDoc);
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT(pXmlObj);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(pXmlNodes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    CPPUNIT_ASSERT(!rAttribute.isEmpty());
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    xmlChar * prop = xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr()));
    OString sAttAbsent = OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath
                         + "' no attribute '" + rAttribute + "' exist";
    CPPUNIT_ASSERT_MESSAGE(sAttAbsent.getStr(), prop);
    OUString s(convert(prop));
    xmlFree(prop);
    xmlXPathFreeObject(pXmlObj);
    return s;
}

OUString XmlTestTools::getXPathContent(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    switch (pXmlObj->type)
    {
        case XPATH_UNDEFINED:
            CPPUNIT_FAIL("Undefined XPath type");
        case XPATH_NODESET:
        {
            xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;

            CPPUNIT_ASSERT_MESSAGE(
                OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' not found")
                    .getStr(),
                xmlXPathNodeSetGetLength(pXmlNodes) > 0);

            xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
            xmlNodePtr pXmlChild = pXmlNode->children;
            OUString s;
            while (pXmlChild && pXmlChild->type != XML_TEXT_NODE)
                pXmlChild = pXmlChild->next;
            if (pXmlChild && pXmlChild->type == XML_TEXT_NODE)
                s = convert(pXmlChild->content);
            xmlXPathFreeObject(pXmlObj);
            return s;
        }
        case XPATH_BOOLEAN:
            {
                auto boolVal = pXmlObj->boolval;
                xmlXPathFreeObject(pXmlObj);
                return boolVal ? OUString("true") : OUString("false");
            }
        case XPATH_NUMBER:
            {
                auto floatVal = pXmlObj->floatval;
                xmlXPathFreeObject(pXmlObj);
                return OUString::number(floatVal);
            }
        case XPATH_STRING:
            {
                auto convertedVal = convert(pXmlObj->stringval);
                xmlXPathFreeObject(pXmlObj);
                return convertedVal;
            }
#if LIBXML_VERSION < 21000 || defined(LIBXML_XPTR_LOCS_ENABLED)
        case XPATH_POINT:
        case XPATH_RANGE:
        case XPATH_LOCATIONSET:
#endif
        case XPATH_USERS:
        case XPATH_XSLT_TREE:
            xmlXPathFreeObject(pXmlObj);
            CPPUNIT_FAIL("Unsupported XPath type");
    }

    CPPUNIT_FAIL("Invalid XPath type");
}

void XmlTestTools::assertXPath(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OString& rAttribute, const OUString& rExpectedValue)
{
    OUString aValue = getXPath(pXmlDoc, rXPath, rAttribute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, attribute '" + rAttribute + "' of '" + rXPath + "' incorrect value.").getStr(),
                                 rExpectedValue, aValue);
}

void XmlTestTools::assertXPathDoubleValue(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OString& rAttribute, double expectedValue, double delta)
{
    OUString aValue = getXPath(pXmlDoc, rXPath, rAttribute);
    double pathValue = aValue.toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(std::string("In <") + std::string(pXmlDoc->name ? pXmlDoc->name : "") + ">, attribute '" + std::string(rAttribute) + "' of '" + std::string(rXPath) + "' incorrect value.",
                                         expectedValue, pathValue, delta);
}

void XmlTestTools::assertXPathAttrs(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath,
                                    const std::vector<std::pair<OString, OUString>>& aPairVector)
{
    for (auto& rPair : aPairVector)
    {
        assertXPath(pXmlDoc, rXPath, rPair.first, rPair.second);
    }
}

int XmlTestTools::countXPathNodes(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    const int n = xmlXPathNodeSetGetLength(pXmlNodes);
    xmlXPathFreeObject(pXmlObj);
    return n;
}

void XmlTestTools::assertXPath(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, int nNumberOfNodes)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 nNumberOfNodes, countXPathNodes(pXmlDoc, rXPath));
}

void XmlTestTools::assertXPathContent(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OUString& rContent)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath contents of child does not match").getStr(), rContent, getXPathContent(pXmlDoc, rXPath));
}

void XmlTestTools::assertXPathNSDef(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath,
                                    std::u16string_view rNSPrefix, std::u16string_view rNSHref)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_MESSAGE(
        OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' not found").getStr(),
        xmlXPathNodeSetGetLength(pXmlNodes) > 0);

    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    bool bFound = false;
    for (xmlNsPtr pNamespace = pXmlNode->nsDef; pNamespace; pNamespace = pNamespace->next)
    {
        if (!pNamespace->prefix)
            continue;

        CPPUNIT_ASSERT(pNamespace->href);
        if (rNSPrefix == convert(pNamespace->prefix) && rNSHref == convert(pNamespace->href))
        {
            bFound = true;
            break;
        }
    }
    xmlXPathFreeObject(pXmlObj);
    CPPUNIT_ASSERT(bFound);
}

void XmlTestTools::assertXPathChildren(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, int nNumberOfChildNodes)
{
#if LIBXML_VERSION >= 20703 /* xmlChildElementCount is only available in libxml2 >= 2.7.3 */
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(pXmlNodes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of child-nodes is incorrect").getStr(),
                                 nNumberOfChildNodes, static_cast<int>(xmlChildElementCount(pXmlNode)));
    xmlXPathFreeObject(pXmlObj);
#else
    (void)pXmlDoc;
    (void)rXPath;
    (void)nNumberOfChildNodes;
#endif
}

void XmlTestTools::assertXPathNoAttribute(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OString& rAttribute)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(pXmlNodes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' unexpected '" + rAttribute + "' attribute").getStr(),
                                 !xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr())));
    xmlXPathFreeObject(pXmlObj);
}

int XmlTestTools::getXPathPosition(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, std::string_view rChildName)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(pXmlNodes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1,
                                 xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    int nRet = 0;
    bool bFound = false;
    for (xmlNodePtr pChild = pXmlNode->children; pChild; pChild = pChild->next)
    {
        if (oconvert(pChild->name) == rChildName)
        {
            bFound = true;
            break;
        }
        ++nRet;
    }
    xmlXPathFreeObject(pXmlObj);
    CPPUNIT_ASSERT_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath
                                   + "' child '" + rChildName + "' not found")
                               .getStr(),
        bFound);
    return nRet;
}

void XmlTestTools::assertXPathNodeName(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath,
                                          const OString& rExpectedName)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT(pXmlObj);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT(pXmlNodes);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1,
                                 xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OString::Concat("In XPath '" + rXPath + "' name of node is incorrect")).getStr(),
                                 rExpectedName, oconvert(pXmlNode->name));
    xmlXPathFreeObject(pXmlObj);
}

void XmlTestTools::registerODFNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("manifest"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0"));
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
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("anim"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:animation:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("smil"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:smil-compatible:1.0"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("presentation"),
                       BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:presentation:1.0"));
    // user-defined
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("foo"),
                       BAD_CAST("http://example.com/"));
}

void XmlTestTools::registerOOXMLNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
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
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w15"),
                       BAD_CAST("http://schemas.microsoft.com/office/word/2012/wordml"));
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
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("x"),
                       BAD_CAST("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("r"),
                       BAD_CAST("http://schemas.openxmlformats.org/officeDocument/2006/relationships"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xx"),
                       BAD_CAST("urn:schemas-microsoft-com:office:excel"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xdr"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("x14"),
                       BAD_CAST("http://schemas.microsoft.com/office/spreadsheetml/2009/9/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xm"),
                       BAD_CAST("http://schemas.microsoft.com/office/excel/2006/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("x12ac"),
                       BAD_CAST("http://schemas.microsoft.com/office/spreadsheetml/2011/1/ac"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("o"),
                       BAD_CAST("urn:schemas-microsoft-com:office:office"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("w10"),
                       BAD_CAST("urn:schemas-microsoft-com:office:word"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("p"),
                       BAD_CAST("http://schemas.openxmlformats.org/presentationml/2006/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("p14"),
                       BAD_CAST("http://schemas.microsoft.com/office/powerpoint/2010/main"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("dgm"),
                       BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/diagram"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("c15"),
                       BAD_CAST("http://schemas.microsoft.com/office/drawing/2012/chart"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("xr2"),
                       BAD_CAST("http://schemas.microsoft.com/office/spreadsheetml/2015/revision2"));
    xmlXPathRegisterNs(pXmlXpathCtx, BAD_CAST("asvg"),
                       BAD_CAST("http://schemas.microsoft.com/office/drawing/2016/SVG/main"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
