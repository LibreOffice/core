/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef XML_TEST_TOOLS_H
#define XML_TEST_TOOLS_H

#include <sal/config.h>
#include <test/testdllapi.hxx>

#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <cppunit/TestAssert.h>

class OOO_DLLPUBLIC_TEST XmlTestTools
{
protected:
    XmlTestTools();
    virtual ~XmlTestTools();

    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx);

    xmlNodeSetPtr getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath);
    OUString      getXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute);
    OUString      getXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath);
    int           getXPathPosition(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rChildName);
    void          assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath,
                              const OString& rAttribute = OString(),
                              const OUString& rExpectedValue = OUString());
    void          assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes);
    void          assertXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rContent);
    void          assertXPathChildren(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfChildNodes);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
