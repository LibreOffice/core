#!/usr/bin/env python
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from __future__ import print_function
from xml.dom import minidom
import sys


def getElementsByTagNamesNS(parent, ns, names, ret=minidom.NodeList()):
    for node in parent.childNodes:
        if node.nodeType == minidom.Node.ELEMENT_NODE and node.namespaceURI == ns and node.tagName in names:
            ret.append(node)
        getElementsByTagNamesNS(node, ns, names, ret)
    return ret


def createFastChildContextFromFactory(model):
    print("""uno::Reference<xml::sax::XFastContextHandler> OOXMLFactory::createFastChildContextFromFactory
(OOXMLFastContextHandler* pHandler, OOXMLFactory_ns::Pointer_t pFactory, Token_t Element)
{
    uno::Reference <xml::sax::XFastContextHandler> aResult;
    Id nDefine = pHandler->getDefine();

    if (pFactory.get() != NULL)
    {
        CreateElementMapPointer pMap = pFactory->getCreateElementMap(nDefine);

        if (pMap.get() != NULL)
        {
            Id nId = pFactory->getResourceId(nDefine, Element);
            CreateElement aCreateElement = (*pMap)[Element];

            switch (aCreateElement.m_nResource)
            {""")
    resources = ["List", "Integer", "Hex", "String", "UniversalMeasure", "Boolean"]
    for resource in [r.getAttribute("resource") for r in model.getElementsByTagName("resource")]:
        if resource not in resources:
            resources.append(resource)
            print("""            case RT_%s:
                aResult.set(OOXMLFastHelper<OOXMLFastContextHandler%s>::createAndSetParentAndDefine(pHandler, Element, nId, aCreateElement.m_nId));
                break;""" % (resource, resource))
    print("""            case RT_Any:
                aResult.set(createFastChildContextFromStart(pHandler, Element));
                break;
            default:
                break;
            }

        }
    }

    return aResult;
}
""")


def getFactoryForNamespace(model):
    print("""OOXMLFactory_ns::Pointer_t OOXMLFactory::getFactoryForNamespace(Id nId)
{
    OOXMLFactory_ns::Pointer_t pResult;

    switch (nId & 0xffff0000)
    {""")

    for namespace in [ns.getAttribute("name") for ns in model.getElementsByTagName("namespace")]:
        id = namespace.replace('-', '_')
        print("""    case NN_%s:
        pResult = OOXMLFactory_%s::getInstance();
        break;""" % (id, id))
    print("""    default:
        break;
    }

    return pResult;
}
""")


def createFastChildContextFromStart(model):
    print("""uno::Reference<xml::sax::XFastContextHandler> OOXMLFactory::createFastChildContextFromStart
(OOXMLFastContextHandler* pHandler, Token_t Element)
{
    uno::Reference<xml::sax::XFastContextHandler> aResult;
    OOXMLFactory_ns::Pointer_t pFactory;

""")

    for namespace in [ns.getAttribute("name") for ns in model.getElementsByTagName("namespace")]:
        id = namespace.replace('-', '_')
        print("""    if (!aResult.is())
    {
        pFactory = getFactoryForNamespace(NN_%s);
        aResult.set(createFastChildContextFromFactory(pHandler, pFactory, Element));
    }""" % id)

    print("""
    return aResult;
}
""")


def fastTokenToId(model):
    print("""namespace tokenmap {
struct token { const char* name; Token_t nToken; };
class Perfect_Hash
{
private:
  static inline unsigned int hash (const char* str, unsigned int len);
public:
  static struct token* in_word_set (const char* str, unsigned int len);
};
}

#ifdef DEBUG_DOMAINMAPPER
string fastTokenToId(sal_uInt32 nToken)
{

    string sResult;

    switch (nToken & 0xffff0000)
    {""")

    aliases = []
    for alias in [a.getAttribute("alias") for a in model.getElementsByTagName("namespace-alias")]:
        if not alias in aliases:
            aliases.append(alias)
            print("""    case NS_%s:
        sResult += "%s:";
        break;""" % (alias, alias))
    print("""    }

    switch (nToken & 0xffff)
    {""")

    tokens = [""]
    for token in [t.getAttribute("localname") for t in getElementsByTagNamesNS(model, "http://relaxng.org/ns/structure/1.0", ["element", "attribute"])]:
        if not token in tokens:
            tokens.append(token)
            print("""    case oox::XML_%s:
        sResult += "%s";
        break;""" % (token, token))

    print("""    }

    return sResult;
}
#endif
""")


def getFastParser(model):
    print("""uno::Reference <xml::sax::XFastParser> OOXMLStreamImpl::getFastParser()
{
    if (!mxFastParser.is())
    {
        mxFastParser = css::xml::sax::FastParser::create(mxContext);
""")
    for alias in model.getElementsByTagName("namespace-alias"):
        print("""        mxFastParser->registerNamespace("%s", NS_%s);""" % (alias.getAttribute("name"), alias.getAttribute("alias")))
    print("""    }

    return mxFastParser;
}

/// @endcond
}}""")


def createImpl(model):
    print("""
#include <com/sun/star/xml/sax/FastParser.hpp>
#include "ooxml/OOXMLFactory.hxx"
#include "ooxml/OOXMLFastHelper.hxx"
#include "ooxml/OOXMLStreamImpl.hxx"
""")

    for namespace in [ns.getAttribute("name") for ns in model.getElementsByTagName("namespace")]:
        print('#include "OOXMLFactory_%s.hxx"' % namespace)

    print("""namespace writerfilter {
namespace ooxml {

/// @cond GENERATED
""")

    createFastChildContextFromFactory(model)
    getFactoryForNamespace(model)
    createFastChildContextFromStart(model)
    fastTokenToId(model)
    getFastParser(model)


modelPath = sys.argv[1]
model = minidom.parse(modelPath)
createImpl(model)

# vim:set shiftwidth=4 softtabstop=4 expandtab:
