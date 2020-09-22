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

xmlDocUniquePtr XmlTestTools::parseXml(utl::TempFile const & aTempFile)
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

void XmlTestTools::registerNamespaces(xmlXPathContextPtr& /*pXmlXpathCtx*/)
{
}

OUString XmlTestTools::getXPath(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OString& rAttribute)
{
    CPPUNIT_ASSERT(pXmlDoc);
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    if (rAttribute.isEmpty())
    {
        xmlXPathFreeObject(pXmlObj);
        return OUString();
    }
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    xmlChar * prop = xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr()));
    OString sAttAbsent = OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath
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
                OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' not found")
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
        case XPATH_POINT:
        case XPATH_RANGE:
        case XPATH_LOCATIONSET:
        case XPATH_USERS:
        case XPATH_XSLT_TREE:
            xmlXPathFreeObject(pXmlObj);
            CPPUNIT_FAIL("Unsupported XPath type");
    }

    CPPUNIT_FAIL("Invalid XPath type");
}

void XmlTestTools::assertXPath(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath)
{
    getXPath(pXmlDoc, rXPath, ""); // it asserts that rXPath exists, and returns exactly one node
}

void XmlTestTools::assertXPath(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OString& rAttribute, const OUString& rExpectedValue)
{
    OUString aValue = getXPath(pXmlDoc, rXPath, rAttribute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, attribute '" + rAttribute + "' of '" + rXPath + "' incorrect value.").getStr(),
                                 rExpectedValue, aValue);
}

void XmlTestTools::assertXPathAttrs(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath,
                                    const std::vector<std::pair<OString, OUString>>& aPairVector)
{
    for (auto& rPair : aPairVector)
    {
        assertXPath(pXmlDoc, rXPath, rPair.first, rPair.second);
    }
}

void XmlTestTools::assertXPath(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, int nNumberOfNodes)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 nNumberOfNodes, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

void XmlTestTools::assertXPathContent(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OUString& rContent)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath contents of child does not match").getStr(), rContent, getXPathContent(pXmlDoc, rXPath));
}

void XmlTestTools::assertXPathNSDef(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath,
                                    const OUString& rNSPrefix, const OUString& rNSHref)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_MESSAGE(
        OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' not found").getStr(),
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of child-nodes is incorrect").getStr(),
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' unexpected '" + rAttribute + "' attribute").getStr(),
                                 static_cast<xmlChar*>(nullptr), xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr())));
    xmlXPathFreeObject(pXmlObj);
}

int XmlTestTools::getXPathPosition(const xmlDocUniquePtr& pXmlDoc, const OString& rXPath, const OString& rChildName)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
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
    CPPUNIT_ASSERT_MESSAGE(OString(OStringLiteral("In <") + pXmlDoc->name + ">, XPath '" + rXPath
                                   + "' child '" + rChildName + "' not found")
                               .getStr(),
        bFound);
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
