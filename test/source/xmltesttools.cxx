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

}

XmlTestTools::XmlTestTools()
{}

XmlTestTools::~XmlTestTools()
{}

xmlDocPtr XmlTestTools::parseXml(utl::TempFile& aTempFile)
{
    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    return parseXmlStream(&aFileStream);
}

xmlDocPtr XmlTestTools::parseXmlStream(SvStream* pStream)
{
    sal_Size nSize = pStream->remainingSize();
    std::unique_ptr<sal_uInt8[]> pBuffer(new sal_uInt8[nSize + 1]);
    pStream->Read(pBuffer.get(), nSize);
    pBuffer[nSize] = 0;
    return xmlParseDoc(reinterpret_cast<xmlChar*>(pBuffer.get()));
}

xmlXPathObjectPtr XmlTestTools::getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath)
{
    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
    registerNamespaces(pXmlXpathCtx);
    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(rXPath.getStr()), pXmlXpathCtx);
    xmlXPathFreeContext(pXmlXpathCtx);
    return pXmlXpathObj;
}

void XmlTestTools::registerNamespaces(xmlXPathContextPtr& /*pXmlXpathCtx*/)
{
}

OUString XmlTestTools::getXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    if (rAttribute.isEmpty())
        return OUString();
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    xmlChar * prop = xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr()));
    OUString s(convert(prop));
    xmlFree(prop);
    xmlXPathFreeObject(pXmlObj);
    return s;
}

OUString XmlTestTools::getXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;

    CPPUNIT_ASSERT_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath '" + rXPath + "' not found").getStr(),
            xmlXPathNodeSetGetLength(pXmlNodes) > 0);

    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    OUString s(convert((pXmlNode->children[0]).content));
    xmlXPathFreeObject(pXmlObj);
    return s;
}

void XmlTestTools::assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute, const OUString& rExpectedValue)
{
    OUString aValue = getXPath(pXmlDoc, rXPath, rAttribute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, attribute '" + rAttribute + "' of '" + rXPath + "' incorrect value.").getStr(),
                                 rExpectedValue, aValue);
}

void XmlTestTools::assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 nNumberOfNodes, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlXPathFreeObject(pXmlObj);
}

void XmlTestTools::assertXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rContent)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath contents of child does not match").getStr(), rContent, getXPathContent(pXmlDoc, rXPath));
}

void XmlTestTools::assertXPathChildren(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfChildNodes)
{
#if LIBXML_VERSION >= 20703 /* xmlChildElementCount is only available in libxml2 >= 2.7.3 */
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath '" + rXPath + "' number of child-nodes is incorrect").getStr(),
                                 nNumberOfChildNodes, (int)xmlChildElementCount(pXmlNode));
    xmlXPathFreeObject(pXmlObj);
#else
    (void)pXmlDoc;
    (void)rXPath;
    (void)nNumberOfChildNodes;
#endif
}

void XmlTestTools::assertXPathNoAttribute(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath '" + rXPath + "' unexpected '" + rAttribute + "' attribute").getStr(),
                                 static_cast<xmlChar*>(nullptr), xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr())));
    xmlXPathFreeObject(pXmlObj);
}

int XmlTestTools::getXPathPosition(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rChildName)
{
    xmlXPathObjectPtr pXmlObj = getXPathNode(pXmlDoc, rXPath);
    xmlNodeSetPtr pXmlNodes = pXmlObj->nodesetval;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("In <" + OString(pXmlDoc->name) + ">, XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1,
                                 xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    int nRet = 0;
    for (xmlNodePtr pChild = pXmlNode->children; pChild; pChild = pChild->next)
    {
        if (convert(pChild->name) == rChildName)
            break;
        ++nRet;
    }
    xmlXPathFreeObject(pXmlObj);
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
