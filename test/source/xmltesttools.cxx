/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/xmltesttools.hxx>

XmlTestTools::XmlTestTools()
{}

XmlTestTools::~XmlTestTools()
{}

xmlNodeSetPtr XmlTestTools::getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath)
{
    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
    registerNamespaces(pXmlXpathCtx);
    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(rXPath.getStr()), pXmlXpathCtx);
    return pXmlXpathObj->nodesetval;
}

void XmlTestTools::registerNamespaces(xmlXPathContextPtr& /*pXmlXpathCtx*/)
{
}

/**
 * Same as the assertXPath(), but don't assert: return the string instead.
 */
OUString XmlTestTools::getXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    if (rAttribute.isEmpty())
        return OUString();
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    return OUString::createFromAscii((const char*)xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr())));
}

/**
 * Same as the assertXPathContent(), but don't assert: return the string instead.
 */
OUString XmlTestTools::getXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);

    CPPUNIT_ASSERT_MESSAGE(OString("XPath '" + rXPath + "' not found").getStr(),
            xmlXPathNodeSetGetLength(pXmlNodes) > 0);

    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    return OUString::createFromAscii((const char*)((pXmlNode->children[0]).content));
}

/**
 * Assert that rXPath exists, and returns exactly one node.
 * In case rAttribute is provided, the rXPath's attribute's value must
 * equal to the rExpected value.
 */
void XmlTestTools::assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute, const OUString& rExpectedValue)
{
    OUString aValue = getXPath(pXmlDoc, rXPath, rAttribute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("Attribute '" + rAttribute + "' of '" + rXPath + "' incorrect value.").getStr(),
                                 rExpectedValue, aValue);
}

/**
 * Assert that rXPath exists, and returns exactly nNumberOfNodes nodes.
 * Useful for checking that we do _not_ export some node (nNumberOfNodes == 0).
 */
void XmlTestTools::assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 nNumberOfNodes, xmlXPathNodeSetGetLength(pXmlNodes));
}

/**
 * Assert that rXPath exists, and its content equals rContent.
 */
void XmlTestTools::assertXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rContent)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("XPath contents of child does not match", rContent, getXPathContent(pXmlDoc, rXPath));
}

/**
 * Assert that rXPath exists, and has exactly nNumberOfChildNodes child nodes.
 * Useful for checking that we do have a no child nodes to a specific node (nNumberOfChildNodes == 0).
 */
void XmlTestTools::assertXPathChildren(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfChildNodes)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1, xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("XPath '" + rXPath + "' number of child-nodes is incorrect").getStr(),
                                 nNumberOfChildNodes, (int)xmlChildElementCount(pXmlNode));
}

/**
 * Get the position of the child named rName of the parent node specified by rXPath.
 * Useful for checking relative order of elements.
 */
int XmlTestTools::getXPathPosition(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rChildName)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
                                 1,
                                 xmlXPathNodeSetGetLength(pXmlNodes));
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    int nRet = 0;
    for (xmlNodePtr pChild = pXmlNode->children; pChild; pChild = pChild->next)
    {
        if (OUString::createFromAscii((const char*)pChild->name) == rChildName)
            break;
        ++nRet;
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
