/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_XMLTESTTOOLS_HXX
#define INCLUDED_TEST_XMLTESTTOOLS_HXX

#include <sal/config.h>
#include <test/testdllapi.hxx>

#include <libxml/xmlwriter.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/mtfxmldump.hxx>
#include <test/xmldocptr.hxx>

#include <cppunit/TestAssert.h>

#include <string_view>
#include <vector>

class OOO_DLLPUBLIC_TEST XmlTestTools
{
public:
    /// Return xmlDocPtr representation of the XML stream read from pStream.
    static xmlDocUniquePtr parseXmlStream(SvStream* pStream);

    static xmlDocUniquePtr dumpAndParse(MetafileXmlDump& rDumper, const GDIMetaFile& rGDIMetaFile);

protected:
    XmlTestTools();
    virtual ~XmlTestTools();

    static xmlDocUniquePtr parseXml(utl::TempFileNamed const & aTempFile);

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx);

    // Caller must call xmlXPathFreeObject:
    xmlXPathObjectPtr getXPathNode(const xmlDocUniquePtr& pXmlDoc, const char* pXPath);
    /**
     * Same as the assertXPath(), but don't assert: return the string instead.
     */
    OUString      getXPath(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, const char* pAttribute);
    OUString getXPath(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath, const char* pAttribute)
    {
        return getXPath(pXmlDoc, sXPath.getStr(), pAttribute);
    }
    /**
     * Same as above, but where the expected number of nodes with
     * the given path is nNumPaths and the desired node index is nPathIdx.
     */
    OUString      getXPath(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, int nNumPaths, int nPathIdx, const char* pAttribute);
    OUString getXPath(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath, int nNumPaths, int nPathIdx, const char* pAttribute)
    {
        assert(nPathIdx < nNumPaths);
        return getXPath(pXmlDoc, sXPath.getStr(), nNumPaths, nPathIdx, pAttribute);
    }
    /**
     * Same as the assertXPathContent(), but don't assert: return the string instead.
     */
    OUString      getXPathContent(const xmlDocUniquePtr& pXmlDoc, const char* pXPath);
    OUString getXPathContent(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath)
    {
        return getXPathContent(pXmlDoc, sXPath.getStr());
    }
    /**
     * Get the position of the child named rName of the parent node specified by pXPath.
     * Useful for checking relative order of elements.
     */
    int           getXPathPosition(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, const char* pChildName);
    /**
     * Get the number of the nodes returned by the pXPath.
     */
    int           countXPathNodes(const xmlDocUniquePtr& pXmlDoc, const char* pXPath);
    int countXPathNodes(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath)
    {
        return countXPathNodes(pXmlDoc, sXPath.getStr());
    }
    /**
     * Assert that pXPath exists, returns exactly one node, and the pXPath's attribute's value
     * equals to the rExpected value.
     */
    void          assertXPath(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, const char* pAttribute,
                              std::u16string_view rExpectedValue);
    void assertXPath(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath, const char* pAttribute,
                     std::u16string_view rExpectedValue)
    {
        assertXPath(pXmlDoc, sXPath.getStr(), pAttribute, rExpectedValue);
    }
    void          assertXPathAttrs(const xmlDocUniquePtr& pXmlDoc, const char* pXPath,
                          const std::vector<std::pair<const char*, std::u16string_view>>& aPairVector);
    void assertXPathAttrs(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath,
                     const std::vector<std::pair<const char*, std::u16string_view>>& aPairVector)
    {
        assertXPathAttrs(pXmlDoc, sXPath.getStr(), aPairVector);
    }
    /**
     * Assert that pXPath exists, returns nNumNodes nodes, and the attribute
     * value of node number nPathIdx equals the rExpected value.
     */
    void          assertXPath(const xmlDocUniquePtr& pXmlDoc, const char* pXPath,
                              int nNumNodes, int nNodeIdx,
                              const char* pAttribute, std::u16string_view rExpectedValue);
    void assertXPath(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath,
                     int nNumNodes, int nNodeIdx,
                     const char* pAttribute, std::u16string_view rExpectedValue)
    {
        assertXPath(pXmlDoc, sXPath.getStr(), nNumNodes, nNodeIdx, pAttribute, rExpectedValue);
    }

    /**
     * Given a double for the rExpected value, assert that pXPath exists, returns exactly one node,
     * and the pXPath's attribute's value matches the rExpected value within tolerance provided
     * by delta. This is used to account for HiDPI scaling.
     */
    void          assertXPathDoubleValue(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, const char* pAttribute,
                              double expectedValue, double delta);
    /**
     * Assert that pXPath exists, and returns exactly nNumberOfNodes nodes (1 by default).
     * Also useful for checking that we do _not_ export some node (nNumberOfNodes == 0).
     */
    void          assertXPath(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, int nNumberOfNodes = 1);
    void assertXPath(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath, int nNumberOfNodes = 1)
    {
        assertXPath(pXmlDoc, sXPath.getStr(), nNumberOfNodes);
    }
    /**
     * Assert that pXPath exists, and its content equals rContent.
     */
    void          assertXPathContent(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, std::u16string_view rContent);
    void assertXPathContent(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath,
                            std::u16string_view rContent)
    {
        assertXPathContent(pXmlDoc, sXPath.getStr(), rContent);
    }
    /**
     * Assert that pXPath exists and it has an rNSPrefix=rNSHref namespace definition.
     */
    void          assertXPathNSDef(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, std::string_view rNSPrefix,
                          std::string_view rNSHref);
    /**
     * Assert that pXPath exists, and has exactly nNumberOfChildNodes child nodes.
     * Useful for checking that we do have a no child nodes to a specific node (nNumberOfChildNodes == 0).
     */
    void          assertXPathChildren(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, int nNumberOfChildNodes);
    void assertXPathChildren(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath,
                             int nNumberOfChildNodes)
    {
        assertXPathChildren(pXmlDoc, sXPath.getStr(), nNumberOfChildNodes);
    }
    /**
     * Assert that pXPath exists, has exactly 1 result set nodes and does *not* have an attribute named pAttribute.
     */
    void          assertXPathNoAttribute(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, const char* pAttribute);
    void assertXPathNoAttribute(const xmlDocUniquePtr& pXmlDoc, const OString& sXPath,
                                const char* pAttribute)
    {
        assertXPathNoAttribute(pXmlDoc, sXPath.getStr(), pAttribute);
    }
    /**
     * Same as the assertXPathNoAttribute(), but don't assert: return the bool instead.
     */
    bool          hasXPathAttribute(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, const char* pAttribute);
    // Assert that the node name of the single node returned by an XPath is as specified,
    // e.g. to check order of elements, where getXPathPosition is unapplicable
    void assertXPathNodeName(const xmlDocUniquePtr& pXmlDoc, const char* pXPath, std::string_view rExpectedName);

    static void registerODFNamespaces(xmlXPathContextPtr& pXmlXpathCtx);
    static void registerOOXMLNamespaces(xmlXPathContextPtr& pXmlXpathCtx);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
