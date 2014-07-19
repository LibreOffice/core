/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xpath.hxx"

#include <cppunit/TestAssert.h>
#include "qahelper.hxx"

#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

xmlDocPtr XPathHelper::parseExport(ScDocShell* pShell, uno::Reference<lang::XMultiServiceFactory> xSFactory, const OUString& rFile, sal_Int32 nFormat)
{
    boost::shared_ptr<utl::TempFile> pTempFile = ScBootstrapFixture::exportTo(pShell, nFormat);

    // Read the XML stream we're interested in.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(xSFactory), pTempFile->GetURL());
    uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(rFile), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xInputStream.is());
    boost::shared_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    sal_uInt64 const nSize = pStream->remainingSize();
    OStringBuffer aDocument(nSize);
    char ch;
    for (sal_Size i = 0; i < nSize; ++i)
    {
        pStream->ReadChar( ch );
        aDocument.append(ch);
    }
    CPPUNIT_ASSERT(!aDocument.isEmpty());

    // Parse the XML.
    return xmlParseMemory((const char*)aDocument.getStr(), aDocument.getLength());
}

xmlNodeSetPtr XPathHelper::getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath)
{
    struct { xmlChar* pPrefix; xmlChar* pURI; } aNamespaces[] =
    {
        { BAD_CAST("w"), BAD_CAST("http://schemas.openxmlformats.org/wordprocessingml/2006/main") },
        { BAD_CAST("v"), BAD_CAST("urn:schemas-microsoft-com:vml") },
        { BAD_CAST("c"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/chart") },
        { BAD_CAST("a"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/main") },
        { BAD_CAST("mc"), BAD_CAST("http://schemas.openxmlformats.org/markup-compatibility/2006") },
        { BAD_CAST("wps"), BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingShape") },
        { BAD_CAST("wpg"), BAD_CAST("http://schemas.microsoft.com/office/word/2010/wordprocessingGroup") },
        { BAD_CAST("wp"), BAD_CAST("http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing") },
        { BAD_CAST("office"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:office:1.0") },
        { BAD_CAST("table"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:table:1.0") },
        { BAD_CAST("text"), BAD_CAST("urn:oasis:names:tc:opendocument:xmlns:text:1.0") },
        { BAD_CAST("xlink"), BAD_CAST("http://www.w3c.org/1999/xlink") }
    };
    xmlXPathContextPtr pXmlXpathCtx = xmlXPathNewContext(pXmlDoc);
    for(size_t i = 0; i < SAL_N_ELEMENTS(aNamespaces); ++i)
    {
        xmlXPathRegisterNs(pXmlXpathCtx, aNamespaces[i].pPrefix, aNamespaces[i].pURI );
    }

    xmlXPathObjectPtr pXmlXpathObj = xmlXPathEvalExpression(BAD_CAST(rXPath.getStr()), pXmlXpathCtx);
    return pXmlXpathObj->nodesetval;
}

void XPathHelper::assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute, const OUString& rExpectedValue)
{
    OUString aValue = getXPath(pXmlDoc, rXPath, rAttribute);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("Attribute '" + rAttribute + "' of '" + rXPath + "' incorrect value.").getStr(),
            rExpectedValue, aValue);
}

void XPathHelper::assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
            nNumberOfNodes, xmlXPathNodeSetGetLength(pXmlNodes));
}

void XPathHelper::assertXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rContent)
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("XPath contents of child does not match", rContent, getXPathContent(pXmlDoc, rXPath));
}

OUString XPathHelper::getXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);

    CPPUNIT_ASSERT_MESSAGE(OString("XPath '" + rXPath + "' not found").getStr(),
            xmlXPathNodeSetGetLength(pXmlNodes) > 0);

    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];

    _xmlNode *pNode = &(pXmlNode->children[0]);
    return pNode ? OUString::createFromAscii((const char*)((pXmlNode->children[0]).content)) : OUString();
}

OUString XPathHelper::getXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute)
{
    xmlNodeSetPtr pXmlNodes = getXPathNode(pXmlDoc, rXPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("XPath '" + rXPath + "' number of nodes is incorrect").getStr(),
            1, xmlXPathNodeSetGetLength(pXmlNodes));
    if (rAttribute.isEmpty())
        return OUString();
    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
    return OUString::createFromAscii((const char*)xmlGetProp(pXmlNode, BAD_CAST(rAttribute.getStr())));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
