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

#include <cppunit/TestAssert.h>

#include <vector>

class OOO_DLLPUBLIC_TEST XmlTestTools
{
public:
    /// Return xmlDocPtr representation of the XML stream read from pStream.
    static xmlDocPtr parseXmlStream(SvStream* pStream);

protected:
    XmlTestTools();
    virtual ~XmlTestTools();

    static xmlDocPtr parseXml(utl::TempFile const & aTempFile);

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx);

    // Caller must call xmlXPathFreeObject:
    xmlXPathObjectPtr getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath);
    /**
     * Same as the assertXPath(), but don't assert: return the string instead.
     */
    OUString      getXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute);
    /**
     * Same as the assertXPathContent(), but don't assert: return the string instead.
     */
    OUString      getXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath);
    /**
     * Get the position of the child named rName of the parent node specified by rXPath.
     * Useful for checking relative order of elements.
     */
    int           getXPathPosition(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rChildName);
    /**
     * Assert that rXPath exists, and returns exactly one node.
     * In case rAttribute is provided, the rXPath's attribute's value must
     * equal to the rExpected value.
     */
    void          assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath,
                              const OString& rAttribute = OString(),
                              const OUString& rExpectedValue = OUString());
    void          assertXPathAttrs(xmlDocPtr pXmlDoc, const OString& rXPath,
                                   std::vector<std::pair<OString, OUString>> aPairVector);
    /**
     * Assert that rXPath exists, and returns exactly nNumberOfNodes nodes.
     * Useful for checking that we do _not_ export some node (nNumberOfNodes == 0).
     */
    void          assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes);
    /**
     * Assert that rXPath exists, and its content equals rContent.
     */
    void          assertXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rContent);
    /**
     * Assert that rXPath exists and it has an rNSPrefix=rNSHref namespace definition.
     */
    void assertXPathNSDef(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rNSPrefix,
                          const OUString& rNSHref);
    /**
     * Assert that rXPath exists, and has exactly nNumberOfChildNodes child nodes.
     * Useful for checking that we do have a no child nodes to a specific node (nNumberOfChildNodes == 0).
     */
    void          assertXPathChildren(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfChildNodes);
    /**
     * Assert that rXPath exists, has exactly 1 child node and does *not* have an attribute named rAttribute.
     */
    void          assertXPathNoAttribute(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
