/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/XPath.hxx>

namespace tools
{
namespace
{
OUString convert(xmlChar const* sXmlString)
{
    OUString sString;
    rtl_convertStringToUString(&sString.pData, reinterpret_cast<char const*>(sXmlString),
                               xmlStrlen(sXmlString), RTL_TEXTENCODING_UTF8,
                               RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR
                                   | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                                   | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR);
    return sString;
}
}

std::string_view XmlElement::name() { return reinterpret_cast<const char*>(mpXmlNode->name); }

OUString XmlElement::attribute(const char* pAttribute)
{
    OUString sString;

    xmlChar* pProperty = xmlGetProp(mpXmlNode, BAD_CAST(pAttribute));
    if (pProperty)
    {
        sString = convert(pProperty);
        xmlFree(pProperty);
    }
    return sString;
}

sal_Int32 XmlElement::countChildren() { return sal_Int32(xmlChildElementCount(mpXmlNode)); }

std::unique_ptr<XmlElement> XmlElement::at(sal_Int32 nIndex)
{
    sal_Int32 nCurrent = 0;
    xmlNodePtr pCurrent = xmlFirstElementChild(mpXmlNode);

    while (pCurrent != nullptr && nCurrent < nIndex)
    {
        pCurrent = xmlNextElementSibling(pCurrent);
        nCurrent++;
    }

    if (pCurrent)
        return std::make_unique<XmlElement>(pCurrent);
    return {};
}

XPath::XPath(xmlDocPtr pDocPtr)
    : XPath(pDocPtr, [](xmlXPathContextPtr) {})
{
}

XPath::XPath(xmlDocPtr pDocPtr, std::function<void(xmlXPathContextPtr)> funcRegisterNamespaces)
    : mpXmlDocPtr(pDocPtr)
    , mFuncRegisterNamespaces(funcRegisterNamespaces)
{
}

std::unique_ptr<XPathObject> XPath::create(std::string_view aString)
{
    OString aCopy(aString);
    xmlXPathContextPtr pXPathContext = xmlXPathNewContext(mpXmlDocPtr);
    mFuncRegisterNamespaces(pXPathContext);
    xmlXPathObjectPtr pXPathObject
        = xmlXPathEvalExpression(BAD_CAST(aCopy.getStr()), pXPathContext);
    xmlXPathFreeContext(pXPathContext);

    if (!pXPathObject)
        return {};

    if (!pXPathObject->nodesetval)
    {
        xmlXPathFreeObject(pXPathObject);
        return {};
    }

    return std::make_unique<XPathObject>(pXPathObject, aString);
}

std::unique_ptr<XPathObject> XPath::create(std::unique_ptr<XPathObject> const& pPathObject,
                                           std::string_view aString)
{
    return create(Concat2View(pPathObject->getPathString() + OString::Concat(aString)));
}

XPathObject::XPathObject(xmlXPathObjectPtr pXPathObject, std::string_view aString)
    : mpXPathObject(pXPathObject)
    , maPath(aString)
{
}

XPathObject::~XPathObject()
{
    if (mpXPathObject)
        xmlXPathFreeObject(mpXPathObject);
}

sal_Int32 XPathObject::count()
{
    if (!mpXPathObject)
        return 0;

    xmlNodeSetPtr pXmlNodes = mpXPathObject->nodesetval;
    if (!pXmlNodes)
        return 0;

    return xmlXPathNodeSetGetLength(pXmlNodes);
}

OUString XPathObject::attribute(const char* pAttribute)
{
    auto pXmlElement = at(0);
    if (!pXmlElement)
        return OUString();

    return pXmlElement->attribute(pAttribute);
}

std::unique_ptr<XmlElement> XPathObject::at(sal_Int32 nIndex)
{
    if (!mpXPathObject)
        return {};

    if (nIndex >= count())
        return {};

    xmlNodeSetPtr pXmlNodes = mpXPathObject->nodesetval;
    if (!pXmlNodes)
        return {};

    xmlNodePtr pXmlNode = pXmlNodes->nodeTab[nIndex];
    return std::make_unique<XmlElement>(pXmlNode);
}

OUString XPathObject::content()
{
    OUString sError;
    if (!mpXPathObject)
        return sError;

    switch (mpXPathObject->type)
    {
        case XPATH_NODESET:
        {
            xmlNodeSetPtr pXmlNodes = mpXPathObject->nodesetval;
            if (count() != 1)
                return sError;

            xmlNodePtr pXmlNode = pXmlNodes->nodeTab[0];
            return convert(xmlNodeGetContent(pXmlNode));
        }
        case XPATH_BOOLEAN:
        {
            auto bBoolVal = mpXPathObject->boolval;
            return bBoolVal ? u"true"_ustr : u"false"_ustr;
        }
        case XPATH_NUMBER:
        {
            auto floatVal = mpXPathObject->floatval;
            return OUString::number(floatVal);
        }
        case XPATH_STRING:
        {
            return convert(mpXPathObject->stringval);
        }

        case XPATH_UNDEFINED:
            // Undefined XPath type
#if LIBXML_VERSION < 21000 || defined(LIBXML_XPTR_LOCS_ENABLED)
        case XPATH_POINT:
        case XPATH_RANGE:
        case XPATH_LOCATIONSET:
#endif
        case XPATH_USERS:
        case XPATH_XSLT_TREE:
            // Unsupported XPath type
            break;
    }
    return sError;
}

} // end tools namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
