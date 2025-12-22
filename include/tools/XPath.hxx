/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <tools/toolsdllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <memory>
#include <functional>

#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>

namespace tools
{
/** Represents an XML element and allows getting attributes, content, child elements. */
class TOOLS_DLLPUBLIC XmlElement final
{
    xmlNodePtr mpXmlNode;

public:
    XmlElement(xmlNodePtr pXmlNode)
        : mpXmlNode(pXmlNode)
    {
    }

    std::string_view name();
    OUString attribute(const char* pAttribute);
    sal_Int32 countChildren();
    std::unique_ptr<XmlElement> at(sal_Int32 nIndex);
};

/** XPath object is a result of XPath evaluation.
 *
 * It contains the path string and allows to check the attributes,
 * content of the result element (if there is only one) or iterates
 * the resulting elements.
 */
class TOOLS_DLLPUBLIC XPathObject final
{
    xmlXPathObjectPtr mpXPathObject;
    std::string_view maPath;

public:
    XPathObject(xmlXPathObjectPtr pXPathObject, std::string_view aString);
    ~XPathObject();
    std::string_view getPathString() { return maPath; }
    sal_Int32 count();
    OUString attribute(const char* pAttribute);
    OUString content();
    std::unique_ptr<XmlElement> at(sal_Int32 nIndex);
};

/** Object that allows to evaluate XPath strings on a XML document. */
class TOOLS_DLLPUBLIC XPath final
{
    xmlDocPtr mpXmlDocPtr;
    std::function<void(xmlXPathContextPtr)> mFuncRegisterNamespaces;

public:
    XPath(xmlDocPtr pDocPtr);
    XPath(xmlDocPtr pDocPtr, std::function<void(xmlXPathContextPtr)> funcRegisterNamespaces);
    std::unique_ptr<XPathObject> create(std::string_view aString);
    std::unique_ptr<XPathObject> create(std::unique_ptr<XPathObject> const& pPathObject,
                                        std::string_view aString);
};

} // end tools namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
